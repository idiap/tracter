/*
 * Copyright 2008 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "SKMLP.h"
#include "XFile.h"

// Leaving Level 2 code zone.
// Coding monkeys with 0XP welcome.
// You should take some time to improve this code
// and send me a better version.
// Then let me take all the credit!!!

#include "TracterObject.h"

namespace Torch {

  static const char *machine_map[] = {"LINEAR", "SHAREDLINEAR", "SIGMOID", "LOGSOFTMAX", 
				     "SOFTMAX", "CONNECTED MACHINE","SUMMACHINE","TANH",
				      "BLASLINEAR", "LOG","BAUMWELCH","EXP","SCALEDLIKELIHOOD",
				      "SKSOFTMAX", "INVALID"}; 


  SKMLP::SKMLP() : GradientMachine(0,0){
    n_layers = 0;
    n_machines_on_layer = NULL;
    machines = NULL;
    machine_infos = NULL;

    current_layer = -1;
    current_machine = -1;

    // NOTE: start_alpha hack_a_donf mode. Comme je le rempli moi-meme,
    // je fous pas frame_size ici.
    start_alpha = new(allocator) Sequence;
    current_alpha_offset = 0;

    addLayer();
  }

  void SKMLP::build(void){
    // Check links
    checkInternalLinks();

    // Outputs... direct connection ?
    if(n_machines_on_layer[n_layers-1] > 1)
      outputs = new(allocator) Sequence(0, n_outputs);
    else
      outputs = machines[n_layers-1][0]->machine->outputs;

    // Beta...
    if(n_machines_on_layer[0] > 1)
      beta = new(allocator) Sequence(0, n_inputs);
    else
      beta = machines[0][0]->machine->beta;

    // Params...
    if (Tracter::sVerbose > 0)
        printf("# SKMLP: Building machine\n");
    for(int l = 0; l < n_layers; l++)
    {
        if (Tracter::sVerbose > 0)
            printf("#   Layer %i\n",l);
        for(int m = 0; m < n_machines_on_layer[l]; m++)
        {
            ConnectedNode *node = machines[l][m];
            if (Tracter::sVerbose > 0)
                if (machine_infos[l][m]->desc == LINEAR ||
                    machine_infos[l][m]->desc == SHAREDLINEAR ||
                    machine_infos[l][m]->desc == CONNECTED ||
                    machine_infos[l][m]->desc == BLASLINEAR )
                    printf("#     Machine %s with %i inputs and %i outputs\n",
                           machine_map[machine_infos[l][m]->desc],
                           node->machine->n_inputs,
                           node->machine->n_outputs);
                else
                    printf("#     Machine %s with %i units\n",
                           machine_map[machine_infos[l][m]->desc],
                           node->machine->n_inputs);

            if(l > 0){
                if(node->n_input_links == 1)
                    node->inputs = node->input_links[0];
                else
                    node->inputs = new(allocator) Sequence(0, node->machine->n_inputs);
            }
	
            node->alpha = new(allocator) Sequence(0, node->machine->n_outputs);
            params->add(node->machine->params);
            der_params->add(node->machine->der_params);
        }
    }

    if (Tracter::sVerbose > 0)
        message("  Total number of parameters: %d", params->n_params);
  }
  
  void SKMLP::addFCL(GradientMachine *machine_, SKMachineType desc_){
    //message("addingFCL %i",n_layers);

    if(n_machines_on_layer[current_layer]){
      addLayer();
    }
  
    addMachine(machine_, desc_);
  
    if(n_layers > 1){
      for(int i = 0; i < n_machines_on_layer[current_layer-1]; i++)
	connectOn(machines[current_layer-1][i]->machine);
    }
  }

  // more recursivaliscious code --- he he, don't you just love it?
  void SKMLP::insertFCL(int offset_layer, GradientMachine *machine_, SKMachineType desc_){
    int insert_layer;
    
    //message("### Current Layer: %i, CurrentMachine: %i.  Target Layer: %i  ###", current_layer, current_machine, offset_layer);

    if (offset_layer < 0)
      insert_layer = offset_layer+n_layers;
    else
      insert_layer = offset_layer;

    //message("Insert layer %i of n layers %i",insert_layer,n_layers);

    if (current_layer == insert_layer && current_machine < 0){
      //message("  Inserting the new layer");
      addFCL(machine_,desc_);
      addLayer();
    }else{ 
      GradientMachine *input_machine, *output_machine;
      SKMachineType output_type;
      bool removed_machines = false;
      bool removed_layer = false;
      bool connected_off = false;
      
      if (current_machine >= 0)
	connectOff(&input_machine);
      else
	input_machine = NULL;

      if (input_machine != NULL){
	//message("  connectOff");
	connected_off = true;
      }else if(n_machines_on_layer[current_layer] != 0){
	//message("  removeMachine");
	removeMachine(&output_machine,&output_type);
	removed_machines = true;
      }else if(current_layer != offset_layer){
	//message("  removeLayer");
	removeLayer();
	removed_layer = true;
      }
 
      //int layer_check = n_layers;
      insertFCL(offset_layer, machine_, desc_);

      //message("### Current Layer: %i, CurrentMachine: %i.  Target Layer: %i  ###", current_layer, current_machine, insert_layer);

      if (connected_off){
	//message("  connectOn");
	if (current_layer > insert_layer +1){
	  connectOn(input_machine);
	}
      }else if (removed_machines){
	//message("  addMachine");
	addMachine(output_machine, output_type);
	if (current_layer == insert_layer +1){
	  //message("  extra connectOn");
	  for(int i = 0; i < n_machines_on_layer[current_layer-1]; i++)
	    connectOn(machines[current_layer-1][i]->machine);
	}
      }else if(removed_layer){
	//message("  addLayer");
	addLayer();
      }
    }
  }

  // more and more recursivealiscious code 
  void SKMLP::removeFCL(int remove_layer_){
    int remove_layer;
    if (remove_layer_ < 0) // ie -1 == remove the last layer
      remove_layer = remove_layer_ + n_layers;
    else
      remove_layer = remove_layer_;

    //message("Insert layer %i of n layers %i",insert_layer,n_layers);

    if (current_layer == remove_layer){
      //message("found the removed FCL");
      if (current_layer > 0)  // if it's the input layer there's nothing to connectOff!
        for(int i = 0; i < n_machines_on_layer[current_layer-1]; i++){
	  //message("connecting off FCMs");
	  connectOff();
	}
      while (n_machines_on_layer[current_layer] > 0){
	//message("removing machines");
	removeMachine();
      }
      //      message("removing layer");
      removeLayer();
      //message("done");

      return;
    }else{ 
      GradientMachine *input_machine = NULL, *output_machine;
      SKMachineType output_type;
      bool removed_machines = false;
      bool removed_layer = false;
      bool connected_off = false;

      if (current_machine >= 0){
	//message("  connectOff");
	connectOff(&input_machine);
	connected_off = true;
      }else if(n_machines_on_layer[current_layer] != 0){
	//message("  removeMachine");
	removeMachine(&output_machine,&output_type);
	removed_machines = true;
      }else if(current_layer != remove_layer){
	//message("  removeLayer");
	removeLayer();
	removed_layer = true;
      }
 
      //int layer_check = n_layers;
      removeFCL(remove_layer);

      //message("### Current Layer: %i, CurrentMachine: %i.  Target Layer: %i  ###", current_layer, current_machine, remove_layer);

      if (connected_off){
	//message("  connectOn");
	if (current_layer > remove_layer +1){
	  connectOn(input_machine);
	}
      }else if (removed_machines){
	//message("  addMachine");
	addMachine(output_machine, output_type);
	if (current_layer == remove_layer + 1 && remove_layer > 0){
	  //message("  extra connectOn");
	  for(int i = 0; i < n_machines_on_layer[current_layer-1]; i++)
	    connectOn(machines[current_layer-1][i]->machine);
	}
      }else if(removed_layer){
	//message("  addLayer");
	addLayer();
      }
    }
  }
   
  /// Add a #machine# on the current layer
  void SKMLP::addMachine(GradientMachine *machine_, SKMachineType desc_, bool hide){

    machine_infos[current_layer] = (SKMachineTypeNode **)allocator->realloc(machine_infos[current_layer], (n_machines_on_layer[current_layer] + 1)*sizeof(SKMachineTypeNode *));
    machine_infos[current_layer][n_machines_on_layer[current_layer]] = (SKMachineTypeNode *)allocator->alloc(sizeof(SKMachineTypeNode));

    SKMachineTypeNode *mtnode = machine_infos[current_layer][n_machines_on_layer[current_layer]];
    mtnode->machine = machine_;
    mtnode->desc = desc_;
    mtnode->hidden = hide;

    machines[current_layer] = (ConnectedNode **)allocator->realloc(machines[current_layer], (n_machines_on_layer[current_layer] + 1)*sizeof(ConnectedNode *));
    machines[current_layer][n_machines_on_layer[current_layer]] = (ConnectedNode *)allocator->alloc(sizeof(ConnectedNode));

    // Initialisations diverses
    ConnectedNode *node = machines[current_layer][n_machines_on_layer[current_layer]];
    node->machine = machine_;
    node->input_links = NULL;
    node->n_input_links = 0;
    node->alpha_links = NULL;
    node->n_alpha_links = 0;
    node->alpha_links_offset = NULL;
    node->n_inputs_check = 0;
    node->inputs = NULL;
    node->alpha = NULL;

    //---

    current_machine = n_machines_on_layer[current_layer];
    n_machines_on_layer[current_layer]++;

    if(current_layer == 0){
      if(n_machines_on_layer[0] > 1){
	if(machine_->n_inputs != n_inputs)
	  error("SKMLP: trying to connect machine of different input size at the first layer");
      }else
	n_inputs = machine_->n_inputs;
    }

    n_outputs += machine_->n_outputs;
    current_alpha_offset = 0;
  }

  /// remove a #machine# on the current layer
  void SKMLP::removeMachine(GradientMachine **machine_, SKMachineType *desc_){

    if (machine_ != NULL){
      *machine_ = machine_infos[current_layer][current_machine]->machine;
    }
    if (desc_ != NULL){
      *desc_ =  machine_infos[current_layer][current_machine]->desc;
    }

    n_outputs -= machine_infos[current_layer][current_machine]->machine->n_outputs;

    if (n_machines_on_layer[current_layer] == 0)
      error("SKMLP: no machines to remove from this layer, try removing the layer first");

    n_machines_on_layer[current_layer]--;

    allocator->free(machine_infos[current_layer][n_machines_on_layer[current_layer]]);
    machine_infos[current_layer] = (SKMachineTypeNode **)allocator->realloc(machine_infos[current_layer], (n_machines_on_layer[current_layer])*sizeof(SKMachineTypeNode *));

    allocator->free(machines[current_layer][n_machines_on_layer[current_layer]]);
    machines[current_layer] = (ConnectedNode **)allocator->realloc(machines[current_layer],
                                                    (n_machines_on_layer[current_layer])*sizeof(ConnectedNode *));

    current_machine = n_machines_on_layer[current_layer] - 1;
    //n_outputs -= (*machine_)->n_outputs;  
    //current_alpha_offset = ;
  }


  /** Connect the last added machine on #machine#.
      Note that #machine# \emph{must} be in a previous layer.
  */
  void SKMLP::connectOn(GradientMachine *machine_){

    if(current_machine < 0)
      error("SKMLP: no machine to connect");

    bool flag = true;
    int l, m = -666;
    for(l = 0; (l < current_layer) && flag; l++){
      for(m = 0; m < n_machines_on_layer[l]; m++){
	if(machines[l][m]->machine == machine_){
	  flag = false;
	  break;
	}
      }
    }
    
    l--;
    //message("connected machine for current layer/machine %i/%i found at %i/%i",current_layer, current_machine, l,m);
    if(flag)
      error("SKMLP: cannot connect your machine");

    ConnectedNode *node = machines[current_layer][current_machine];
    node->input_links = (Sequence **)allocator->realloc(node->input_links, sizeof(Sequence *)*(node->n_input_links+1));
    node->input_links[node->n_input_links] = machine_->outputs;
    node->n_inputs_check += machines[l][m]->machine->n_outputs;
    node->n_input_links++;

    node = machines[l][m];
 
    node->alpha_links = (Sequence **)allocator->realloc(node->alpha_links, sizeof(Sequence *)*(node->n_alpha_links+1));
    node->alpha_links[node->n_alpha_links] = machines[current_layer][current_machine]->machine->beta;
    node->alpha_links_offset = (int *)allocator->realloc(node->alpha_links_offset, sizeof(int)*(node->n_alpha_links+1));
    node->alpha_links_offset[node->n_alpha_links] = current_alpha_offset;
    node->n_alpha_links++;

    current_alpha_offset += machine_->n_outputs;

  }

  void SKMLP::connectOff(GradientMachine **machine_){
    if(current_machine < 0)
      error("SKMLP: no machine to disconnect");

    ConnectedNode *node = machines[current_layer][current_machine];
    if (node->n_input_links == 0){ // no more disconnections to do
      if (machine_ != NULL)
	*machine_ = NULL;
      return;
    } 
    node->n_input_links--;

    bool flag = true;
    int l, m = -666;
    for(l = 0; (l < current_layer) && flag; l++){
      for(m = 0; m < n_machines_on_layer[l]; m++){
	if(machines[l][m]->machine->outputs == machines[current_layer][current_machine]->input_links[node->n_input_links]){
	  flag = false;
	  break;
	}
      }
    }
    l--;
    //message("connected machine for current layer/machine %i/%i found at %i/%i",current_layer, current_machine, l,m);
    if (machine_ != NULL)
      *machine_=machines[l][m]->machine;

    if(flag)
      error("SKMLP: cannot disconnect your machine");

    node->input_links = (Sequence **)allocator->realloc(node->input_links, sizeof(Sequence *)*(node->n_input_links));
    node->n_inputs_check -= machines[l][m]->machine->n_outputs;
 
    node = machines[l][m];
    node->n_alpha_links--;

    node->alpha_links = (Sequence **)allocator->realloc(node->alpha_links, sizeof(Sequence *)*(node->n_alpha_links));
    node->alpha_links_offset = (int *)allocator->realloc(node->alpha_links_offset, sizeof(int)*(node->n_alpha_links));
    //current_alpha_offset -= (*machine_)->n_outputs;
 
    //message("    alpha_links %i", node->n_alpha_links);
    //message("    alpha_offset %i",current_alpha_offset);
  }

  /// Add a layer (you don't have to call that for the first layer)
  void SKMLP::addLayer() {    
    machine_infos = (SKMachineTypeNode ***)allocator->realloc(machine_infos, (n_layers+1)*sizeof(SKMachineTypeNode **));
    machine_infos[n_layers] = NULL;
    n_outputs = 0;
    
    if(n_layers > 0){
      if(n_machines_on_layer[n_layers-1] == 0)
	error("SKMLP: one layer without any machine !?!");
    }

    machines = (ConnectedNode ***)allocator->realloc(machines, (n_layers+1)*sizeof(ConnectedNode **));
    n_machines_on_layer = (int *)allocator->realloc(n_machines_on_layer, sizeof(int)*(n_layers+1));
    machines[n_layers] = NULL;
    n_machines_on_layer[n_layers] = 0;
    
    current_layer = n_layers;
    current_machine = -1;

    n_layers++;
  }

  void SKMLP::removeLayer(){
    if ( n_machines_on_layer[current_layer] != 0 || current_layer  == 0){
      error("SKMLP: Can not remove this layer, there are still machines on the current layer or there are no layers to remove.");
    }

    machine_infos = (SKMachineTypeNode ***)allocator->realloc(machine_infos, (n_layers-1)*sizeof(SKMachineTypeNode **));
    machines = (ConnectedNode ***)allocator->realloc(machines, (n_layers-1)*sizeof(ConnectedNode **));
    n_machines_on_layer = (int *)allocator->realloc(n_machines_on_layer, sizeof(int)*(n_layers-1));

    n_layers--;
    current_layer = n_layers-1;
    current_machine =  n_machines_on_layer[current_layer]-1;
  }

  // load the machine AND create the machine topology at the same time -> tricky stuff
  // also included is the ability to FREEZE the machine parameters on loading;
  // and now I also add the option to hide the hidden machines on loading.
  void SKMLP::loadXFile(XFile *file, bool fix, bool hide){
    GradientMachine *new_machine = NULL;
    SKMachineType desc_;
    bool hidden;
#ifdef HAVE_SKORCH
    real *priors = NULL;
    int n_windows_;
#endif
    int n_layers_, *n_machines_on_layer_, n_inputs_, n_outputs_;
    
    file->taggedRead(&desc_,sizeof(int),1,"machine_type");
    if (desc_ != CONNECTED){
      error("SKMLP: I was expecting the machine type to be CONNECTED");
    }
    //message("CONNECTED machine loading");

    file->taggedRead(&n_layers_,sizeof(int),1,"n_layers");
    n_machines_on_layer_ = (int *)allocator->alloc(sizeof(int)*n_layers_);

    // create and load the machines into memory from "file"
    for(int i = 0; i < n_layers_; i++)  {
      file->taggedRead(&n_machines_on_layer_[i],sizeof(int),1,"machines_on_layer");
      //message("%i machines on layer %i",n_machines_on_layer_[i],i);
      for(int m = 0; m < n_machines_on_layer_[i]; m++){
	file->taggedRead(&desc_,sizeof(int),1,"machine_type");
	file->taggedRead(&hidden,sizeof(bool),1,"hidden");
	
	//message("desc_ %i %s",desc_,machine_map[desc_]);

	if (i > 0 && desc_ != BAUMWELCH){
	  addLayer();
	}
	if (!hide || !hidden){ // hide the hidden targets if desired...
	  switch (desc_){
	  case LINEAR:
	    //message("Loading LINEAR machine");
	    file->taggedRead(&n_inputs_,sizeof(int),1,"n_inputs");
	    file->taggedRead(&n_outputs_,sizeof(int),1,"n_outputs");
	    if (!fix){
	      new_machine = new(allocator) Linear(n_inputs_,n_outputs_); 
#ifdef HAVE_SKORCH
	    }else{
	      new_machine = new(allocator) FixedLinear(n_inputs_,n_outputs_);
#endif
	    }
	    break;
	  case LOGSOFTMAX:
	    file->taggedRead(&n_inputs_,sizeof(int),1,"n_units");
	    //message("Loading LOGSOFTMAX machine with %i units", n_inputs_);
	    new_machine = new(allocator) LogSoftMax(n_inputs_);
	    break;
	  case SOFTMAX:
	    file->taggedRead(&n_inputs_,sizeof(int),1,"n_units");
	    //message("Loading LOGSOFTMAX machine with %i units", n_inputs_);
	    new_machine = new(allocator) SoftMax(n_inputs_);
	    break;
	  case TANH:
	    file->taggedRead(&n_inputs_,sizeof(int),1,"n_units");
	    //message("Loading TANH machine with %i units", n_inputs_);
	    new_machine = new(allocator) Tanh(n_inputs_);
	    break;
	  case BLASLINEAR:
	    //message("Loading BLASLINEAR machine");
	    file->taggedRead(&n_inputs_,sizeof(int),1,"n_inputs");
	    file->taggedRead(&n_outputs_,sizeof(int),1,"n_outputs");
	    new_machine = new(allocator) BlasLinear(n_inputs_,n_outputs_); 
	    break;
#ifdef HAVE_SKORCH
	  case SHAREDLINEAR:
	    //message("Loading SHAREDLINEAR machine");
	    file->taggedRead(&n_inputs_,sizeof(int),1,"n_inputs");
	    file->taggedRead(&n_outputs_,sizeof(int),1,"n_outputs");
	    file->taggedRead(&n_windows_,sizeof(int),1,"n_windows");
	    new_machine = new(allocator) SharedLinear(n_inputs_/n_windows_,n_outputs_/n_windows_,n_windows_);
	    break;
	  case EXP:
	    //message("Loading EXP machine");
	    file->taggedRead(&n_inputs_,sizeof(int),1,"n_units");
	    new_machine = new(allocator) Exp(n_inputs_);
	    break;
	  case SIGMOID:
	    //message("Loading SIGMOID machine");
	    file->taggedRead(&n_inputs_,sizeof(int),1,"n_units");
	    new_machine = new(allocator) Sigmoid(n_inputs_);
	    break;
	  case SKSOFTMAX:
	    file->taggedRead(&n_inputs_,sizeof(int),1,"n_units");
	    //message("Loading LOGSOFTMAX machine with %i units", n_inputs_);
	    priors = (real *)allocator->alloc(sizeof(real)*n_inputs_);
	    new_machine = new(allocator) SKSoftMax(n_inputs_, priors);
	    break;
	  case SCALEDLIKELIHOOD:
	    file->taggedRead(&n_inputs,sizeof(int),1,"n_units");
	    priors = (real*)allocator->alloc(sizeof(real)*n_inputs);
	    file->taggedRead(priors,sizeof(real),n_inputs,"priors");
	    new_machine = new(allocator) ScaledLikelihood(n_inputs,priors);
	    break;
	  case BAUMWELCH:
	    //file->taggedRead(&n_inputs_,sizeof(int),1,"n_units");
	    //message("Loading LOGSOFTMAX machine with %i units", n_inputs_);
	    //error("SKMLP: BAUMWELCH layer can not be loaded");
	    //do nothing...
	    new_machine = NULL; //new(allocator) SKBaumWelch(n_inputs_ );
	    //removeLayer(); // and get rid of the last layer
	    break;
#endif
	  case CONNECTED:
	    // Mmmm recursivaliscous....
	    new_machine = new(allocator) SKMLP();	  
	    break;
	  default:
	    error("SKMLP: I don't recognise the machine type.");
	  }

	  // load from the file
	  if (new_machine){
	    if (desc_ != CONNECTED){
	      new_machine->loadXFile(file);
	    }else{
	      ((SKMLP*)new_machine)->loadXFile(file, fix, hide);
	      ((SKMLP*)new_machine)->build();
	      ((SKMLP*)new_machine)->setPartialBackprop();
	    }
	
	    // and then incorporate into the architecture
	    //message("Adding the machine");
	    addMachine(new_machine,desc_,hidden);

	    if(n_layers > 1){
	      for(int j = 0; j < n_machines_on_layer[current_layer-1]; j++)
	        connectOn(machines[current_layer-1][j]->machine);
	    }
	  }
	}
	//	machines[i][m]->machine->loadXFile(file);
      }
      //if (new_machine){
      //	if (i < n_layers_-1){
  	//message("Adding layer");
      //	  addLayer();
      //	}
      //}
    }
  }
  
  // save the MLP including the topology of the connected so that it can be created
  // without a priori knowledge of its topology
  void SKMLP::saveXFile(XFile *file){
    SKMachineType desc_;
    bool hidden;

    // first some basics on the topology
    desc_ = CONNECTED;
    file->taggedWrite(&desc_,sizeof(int),1,"machine_type");
    file->taggedWrite(&n_layers,sizeof(int),1,"n_layers");
    for(int i = 0; i < n_layers; i++){
      file->taggedWrite(&n_machines_on_layer[i],sizeof(int),1,"machines_on_layer");
      for(int m = 0; m < n_machines_on_layer[i]; m++){
	desc_ = machine_infos[i][m]->desc;
	hidden = machine_infos[i][m]->hidden;
	file->taggedWrite(&desc_,sizeof(int),1,"machine_type");
	file->taggedWrite(&hidden,sizeof(bool),1,"hidden");
	GradientMachine *machine = machine_infos[i][m]->machine;
	switch (desc_){
	case BLASLINEAR:
	case LINEAR:
	  file->taggedWrite(&machine->n_inputs,sizeof(int),1,"n_inputs");
	  file->taggedWrite(&machine->n_outputs,sizeof(int),1,"n_outputs");
	  break;
	case EXP:
	case SKSOFTMAX:
	case SIGMOID:
	case SOFTMAX:
	case TANH:
	case LOGSOFTMAX:
	  file->taggedWrite(&machine->n_inputs,sizeof(int),1,"n_units");
	  break;
#ifdef HAVE_SKORCH
	case SCALEDLIKELIHOOD:
	  file->taggedWrite(&machine->n_inputs,sizeof(int),1,"n_units");
	  file->taggedWrite(((ScaledLikelihood*)machine)->priors,sizeof(real),machine->n_inputs,"priors");
	  break;
	case SHAREDLINEAR:
	  file->taggedWrite(&machine->n_inputs,sizeof(int),1,"n_inputs");
	  file->taggedWrite(&machine->n_outputs,sizeof(int),1,"n_outputs");
	  file->taggedWrite(&((SharedLinear*)machine)->n_windows,sizeof(int),1,"n_windows");
	  break;
#endif
	case CONNECTED:
	case BAUMWELCH: // we don't want to save this
	  break;
	default:
	  error("SKMLP: This doesn't appear to be a valid SKMLP file as I couldn't verify the machine type");
	}
	machines[i][m]->machine->saveXFile(file);
      }
    }
  }


  void SKMLP::checkInternalLinks(){
    for(int l = 1; l < n_layers; l++){
      for(int m = 0; m < n_machines_on_layer[l]; m++){
	if(machines[l][m]->machine->n_inputs != machines[l][m]->n_inputs_check)
	  error("SKMLP: incorrect number of inputs for machine [%d %d] (%d instead of %d)", l, m, machines[l][m]->machine->n_inputs, machines[l][m]->n_inputs_check);
      }
    }
  }

  void SKMLP::forward(Sequence *inputs){
    for(int m = 0; m < n_machines_on_layer[0]; m++)
      machines[0][m]->machine->forward(inputs);

    for(int l = 1; l < n_layers; l++){
      for(int m = 0; m < n_machines_on_layer[l]; m++){
	ConnectedNode *node = machines[l][m];

	// NOTE: check for direct input connection
	// NOTE: node->inputs doit etre alors sur node->input_links[0] (sinon sequence vide)
	if(node->n_input_links == 1)
	  node->machine->forward(node->inputs);
	else{
	  int n_frames_ = node->input_links[0]->n_frames;
	  node->inputs->resize(n_frames_);

	  int offset_ = 0;
	  for(int i = 0; i < node->n_input_links; i++){
	    int n_inputs_ = node->input_links[i]->frame_size;
	    for(int j = 0; j < n_frames_; j++){
	      real *dest_ = node->inputs->frames[j] + offset_;
	      real *src_ = node->input_links[i]->frames[j];

	      for(int k = 0; k < n_inputs_; k++)
		dest_[k] = src_[k];
	    }
	    offset_ += n_inputs_;
	  }
	  node->machine->forward(node->inputs);
	}
      }
    }
    // NOTE: if not direct output connection, updates output.
    if(n_machines_on_layer[n_layers-1] > 1){
      int n_frames_ = machines[n_layers-1][0]->machine->outputs->n_frames;
      outputs->resize(n_frames_);

      int offset_ = 0;
      for(int i = 0; i < n_machines_on_layer[n_layers-1]; i++){
	int n_outputs_ = machines[n_layers-1][i]->machine->n_outputs;
	for(int j = 0; j < n_frames_; j++){
	  real *dest_ = outputs->frames[j] + offset_;
	  real *src_ = machines[n_layers-1][i]->machine->outputs->frames[j];
        
	  for(int k = 0; k < n_outputs_; k++)
	    dest_[k] = src_[k];
	}
	offset_ += n_outputs_;
      }
    }
  }

  void SKMLP::backward(Sequence *inputs, Sequence *alpha){
    Sequence *alpha_ = NULL;
    if(n_machines_on_layer[n_layers-1] > 1){
      start_alpha->resize(alpha->n_frames, false);
      start_alpha->frame_size = machines[n_layers-1][0]->machine->n_outputs;
      for(int i = 0; i < alpha->n_frames; i++)
	start_alpha->frames[i] = alpha->frames[i];
      alpha_ = start_alpha;
    }else
      alpha_ = alpha;

    if(n_layers > 1){
      for(int m = 0; m < n_machines_on_layer[n_layers-1]; m++){
	ConnectedNode *node = machines[n_layers-1][m];

	// NOTE: on ne tripote donc pas, en aucun cas, le alpha donne par l'utilisateur...
	// NOTE: dans le truc qui suit c'est bien un +=...
	if(m > 0){
	  int offset_ = machines[n_layers-1][m-1]->machine->n_outputs;
	  alpha_->frame_size = node->machine->n_outputs;
	  for(int i = 0; i < alpha_->n_frames; i++)
	    alpha_->frames[i] += offset_;
	}

	node->machine->backward(node->inputs, alpha_);
      }
    }else{
      for(int m = 0; m < n_machines_on_layer[0]; m++){
	ConnectedNode *node = machines[0][m];

	if(m > 0){
	  // NOTE: on ne tripote donc pas, en aucun cas, le alpha donne par l'utilisateur...
	  // NOTE: dans le truc qui suit c'est bien un +=...
	  int offset_ = machines[0][m-1]->machine->n_outputs;
	  alpha_->frame_size = node->machine->n_outputs;
	  for(int i = 0; i < alpha_->n_frames; i++)
	    alpha_->frames[i] += offset_;
	}

	node->machine->backward(inputs, alpha_);
      }
    }
    // NOTE: on pourrait encore optimiser une copie, mais 'sti...
    //message("layers: %i", n_layers);
    for(int l = n_layers-2; l >= 0; l--){
      //message("machines_on_layer %i", n_machines_on_layer[l]);
      for(int m = 0; m < n_machines_on_layer[l]; m++){
	ConnectedNode *node = machines[l][m];

	// NOTE: n_frames: fournie par alpha_link. Attention offset_alpha.
	// NOTE: taille de alpha fournie par machine->n_outputs;
	// a) Fout la taille de alpha comme il faut et initialise a 0
	int n_frames_ = node->alpha_links[0]->n_frames;
	int size_ = node->machine->n_outputs;
	node->alpha->resize(n_frames_);
	Sequence *alpha_ = node->alpha;
	
	for(int i = 0; i < n_frames_; i++){
	  real *z = alpha_->frames[i];
	  for(int j = 0; j < size_; j++)
	    z[j] = 0;
	}
      
	// b) Fait la putain de somme des CI_MO_NAK d'alphas...
	for(int i = 0; i < node->n_alpha_links; i++){
	  for(int j = 0; j < n_frames_; j++){
	    real *src_ = node->alpha_links[i]->frames[j] + node->alpha_links_offset[i];
	    real *dest_ = alpha_->frames[j];
          for(int k = 0; k < size_; k++)
            dest_[k] += src_[k];
	  }
	}

	// c) backward le boxon
	if(l == 0)
	  machines[0][m]->machine->backward(inputs, alpha_);
	else
	  machines[l][m]->machine->backward(node->inputs, alpha_);
      }
    }

    if( (n_machines_on_layer[0] > 1) && (!partial_backprop) ){
      // a) Fout la taille de beta comme il faut et initialise a 0
      int n_frames_ = machines[0][0]->machine->beta->n_frames;
      beta->resize(n_frames_);

      for(int i = 0; i < n_frames_; i++){
	real *dest_ = beta->frames[i];
	for(int j = 0; j < n_inputs; j++)
	  dest_[j] = 0;
      }
    
      // b) Fait la putain de somme des putains de beta
      for(int i = 0; i < n_machines_on_layer[0]; i++){
	for(int j = 0; j < n_frames_; j++){
	  real *dest_ = beta->frames[j];
	  real *src_ = machines[0][i]->machine->beta->frames[j];
        
	  for(int k = 0; k < n_inputs; k++)
	    dest_[k] += src_[k];
	}
      }
    }
  }

  void SKMLP::reset(){
    for(int i = 0; i < n_layers; i++){
      for(int m = 0; m < n_machines_on_layer[i]; m++)
	machines[i][m]->machine->reset();
    }
  }

  void SKMLP::iterInitialize(){
    for(int i = 0; i < n_layers; i++){
      for(int m = 0; m < n_machines_on_layer[i]; m++)
	machines[i][m]->machine->iterInitialize();
    }  
  }

  void SKMLP::setPartialBackprop(bool flag){
    partial_backprop = flag;
    for(int i = 0; i < n_machines_on_layer[0]; i++)
      machines[0][i]->machine->setPartialBackprop(flag);
  }

  void SKMLP::setDataSet(DataSet *dataset_){
    for(int i = 0; i < n_layers; i++){
      for(int m = 0; m < n_machines_on_layer[i]; m++)
	machines[i][m]->machine->setDataSet(dataset_);
    }
  }

  void SKMLP::frameForward(int t, real *f_inputs, real *f_outputs){
    for(int m = 0; m < n_machines_on_layer[0]; m++){
        machines[0][m]->machine->outputs->resize(1);
        machines[0][m]->machine->frameForward(t, f_inputs, machines[0][m]->machine->outputs->frames[0]);
    }

    for(int l = 1; l < n_layers; l++){
        for(int m = 0; m < n_machines_on_layer[l]; m++){
            ConnectedNode *node = machines[l][m];

            node->machine->outputs->resize(1);
	    if(node->n_input_links == 1)
                node->machine->frameForward(t,node->inputs->frames[0],node->machine->outputs->frames[0]);
            else{
		node->inputs->resize(1);

		int offset_ = 0;
		for(int i = 0; i < node->n_input_links; i++){
                    int n_inputs_ = node->input_links[i]->frame_size;
		     real *dest_ = node->inputs->frames[0] + offset_;
	  	     real *src_ = node->input_links[i]->frames[0];
                     for(int k = 0; k < n_inputs_; k++)
	                dest_[k] = src_[k];
	             offset_ += n_inputs_;
	        }
	        node->machine->frameForward(t,node->inputs->frames[0],node->machine->outputs->frames[0]);
	   }
	}
    }

    //if(n_machines_on_layer[n_layers-1] > 1){
       int offset_ = 0;
       for(int i = 0; i < n_machines_on_layer[n_layers-1]; i++){
          int n_outputs_ = machines[n_layers-1][i]->machine->n_outputs;
          real *dest_ = f_outputs + offset_;
          real *src_ = machines[n_layers-1][i]->machine->outputs->frames[0];
          for(int k = 0; k < n_outputs_; k++)
             dest_[k] = src_[k];
          offset_ += n_outputs_;
       }
    //}

  }

  SKMLP::~SKMLP(){
  }

}
