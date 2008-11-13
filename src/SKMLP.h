#ifndef SK_MLP_INC
#define SK_MLP_INC

#include "ConnectedMachine.h"
#include "Linear.h"
#include "LogSoftMax.h"
#include "Tanh.h"
#include "SoftMax.h"
#ifdef HAVE_SKORCH
#include "FixedLinear.h"
#include "AdaptLinear.h"
#include "SharedLinear.h"
#include "Sigmoid.h"
#include "SumMachine.h"
#include "BlasLinear.h"
#include "Log.h"
#include "SKBaumWelch.h"
#include "Exp.h"
#include "ScaledLikelihood.h"
#include "SKSoftMax.h"
#endif

namespace Torch {
  
  /** Easy connections between several #GradientMachine#.
      GradientMachine has "layers" on which you can
      add some #GradientMachine#.
      
      Derived from the ConnectedMachine class of Ronan Collobert.
      Provides a nice little wrapper for the class so that
      various MLP topologies can be easily implemented without 
      having to specify each topologcal variation in your MAIN program.
      (eg. User defines the topology during contruction/training
      but is able to be ignorant of the topology during testing when
      loading the MLP from disk)

      Also provides functionality for the following:
           Maniptulation of MLP topology (adding, deleting, inserting layers etc.)
	   Fixing the weights of a linear layer (using FixedLinear class)
	   Only supports the machines specified in the "enum" below (if you need to add new ones just ask me!)
           Nesting of connected machines
	   "Hidden targets" for use during training of input-to-hidden units (but not used during testing)

      Maybe other stuff later on, some of this code is no longer used even by me as I was just experimenting
      with it.  Feel free to tell me if it's crap....

      Not necessary restricted to speech but that's what it
      has been written for.

      @author John Dines (john.dines@idiap.ch)
  */

  // type of machine, may be linear, sigmoid etc or may be another "connected machine"
  // beware that I only support these types, so if you want to add other types of layer
  // you'll have to ask me nicely to add these (clearly this class isn't destined for inheritance)
  typedef enum {
    LINEAR = 0,
    LOGSOFTMAX,
    TANH,
    SOFTMAX,
    SHAREDLINEAR,
    SIGMOID,
    SUMMACHINE,
    BLASLINEAR,
    LOG,
    BAUMWELCH,
    EXP,
    SCALEDLIKELIHOOD,
    SKSOFTMAX,
    CONNECTED,
    INVALID
  } SKMachineType;

  // a machine and some descriptive info
  struct SKMachineTypeNode{
    SKMachineType desc;
    GradientMachine *machine;
    bool hidden;
  };

  class SKMLP : public GradientMachine {
  private:
    Sequence *start_alpha;
    int current_alpha_offset;
    int current_layer;
    int current_machine;
    void checkInternalLinks();

    // you need to know what you're doing for the following methods, so I recommend you look at the code before trying to use them
  public:

    /** functions for removing machines - editing the topology of an existing MLP - experimental
	removes the last machine to be added
    **/
    void removeMachine(GradientMachine **machine_ = NULL, SKMachineType *desc_ = NULL);

    /**  Disconnects and returns #machine# that was last machine connected
	 to current machine.
     **/
    void connectOff(GradientMachine **machine_ = NULL);

    /** Removes a layer from the machine, there must be no machines on the current layer
     **/
    void removeLayer();

    /** Add a Full Connected Layer. The #machine# is fully connected
	to the previous layer. If necessary, a layer is added before
	adding the machine.
    **/
    void addFCL(GradientMachine *machine_, SKMachineType desc_);

    // insert a fully connected layer into an existing Connected Architecture
    // >= 0 means insert from the beginning, <= -1 means insert from the end
    void insertFCL(int layer_offset, GradientMachine *machine_, SKMachineType desc_);

    /// Add a #machine# on the current layer
    void addMachine(GradientMachine *machine_, SKMachineType desc_, bool hide = false);

    // removing a FCL, useful if you want to extract outputs at a different layer during testing than from training
    // default is to remove the last fully connected layer
    void removeFCL(int remove_layer_ = -1);

    // these are essentially the old MLP methods, with some extra bells and whistles

    ConnectedNode ***machines;
    SKMachineTypeNode ***machine_infos;
    int *n_machines_on_layer;
    int n_layers;

    SKMLP();
    
    // need to make these calls transparent!!!

    // same as in old connected machine, but layer specific implementations
    void addFCL(SKMLP *machine_){ addFCL(machine_,CONNECTED); }; 
    void addFCL(Linear *machine_){ addFCL(machine_,LINEAR); };
    void addFCL(LogSoftMax *machine_){ addFCL(machine_,LOGSOFTMAX); };
    void addFCL(Tanh *machine_){ addFCL(machine_, TANH); };
    void addFCL(SoftMax *machine_){ addFCL(machine_, SOFTMAX); };
#ifdef HAVE_SKORCH
    void addFCL(FixedLinear *machine_){ addFCL(machine_,LINEAR); };
    void addFCL(AdaptLinear *machine_){ addFCL(machine_,LINEAR); };
    void addFCL(SharedLinear *machine_){ addFCL(machine_,SHAREDLINEAR); };
    void addFCL(Sigmoid *machine_){ addFCL(machine_,SIGMOID); };
    void addFCL(SumMachine *machine_){ addFCL(machine_,SUMMACHINE); };
    void addFCL(BlasLinear *machine_){ addFCL(machine_, BLASLINEAR); };
    void addFCL(Log *machine_){ addFCL(machine_, LOG); };
    void addFCL(Exp *machine_){ addFCL(machine_, EXP); };
    void addFCL(SKBaumWelch *machine_){ addFCL(machine_, BAUMWELCH); };
    void addFCL(ScaledLikelihood *machine_){ addFCL(machine_, SCALEDLIKELIHOOD); };
    void addFCL(SKSoftMax *machine_){ addFCL(machine_, SKSOFTMAX); };
#endif

    // inserting a new layer -- something I wanted to try out for adaptation purposes, but you may not want to
    void insertFCL(int layer_offset, SKMLP *machine_){ insertFCL(layer_offset, machine_,CONNECTED); };
    void insertFCL(int layer_offset, Linear *machine_){ insertFCL(layer_offset, machine_,LINEAR); };
    void insertFCL(int layer_offset, Tanh *machine_){ insertFCL(layer_offset, machine_,TANH); };
    void insertFCL(int layer_offset, LogSoftMax *machine_){ insertFCL(layer_offset, machine_,LOGSOFTMAX); };
    void insertFCL(int layer_offset, SoftMax *machine_){ insertFCL(layer_offset, machine_,SOFTMAX); };
#ifdef HAVE_SKORCH
    void insertFCL(int layer_offset, FixedLinear *machine_){ insertFCL(layer_offset, machine_,LINEAR); };
    void insertFCL(int layer_offset, AdaptLinear *machine_){ insertFCL(layer_offset, machine_,LINEAR); };
    void insertFCL(int layer_offset, SharedLinear *machine_){ insertFCL(layer_offset, machine_,SHAREDLINEAR); };
    void insertFCL(int layer_offset, Sigmoid *machine_){ insertFCL(layer_offset, machine_,SIGMOID); };
    void insertFCL(int layer_offset, SumMachine *machine_){ insertFCL(layer_offset, machine_,SUMMACHINE); };
    void insertFCL(int layer_offset, BlasLinear *machine_){ insertFCL(layer_offset, machine_,BLASLINEAR); };
    void insertFCL(int layer_offset, Log *machine_){ insertFCL(layer_offset, machine_,LOG); };
    void insertFCL(int layer_offset, SKBaumWelch *machine_){ insertFCL(layer_offset, machine_,BAUMWELCH); };
    void insertFCL(int layer_offset, ScaledLikelihood *machine_){ insertFCL(layer_offset, machine_, SCALEDLIKELIHOOD); };
    void insertFCL(int layer_offset, SKSoftMax *machine_){ insertFCL(layer_offset, machine_, SKSOFTMAX); };
#endif

    // once again some functions for handling the accepted machinetypes
    void addMachine(SKMLP *machine_, bool hide = false){ addMachine(machine_,CONNECTED, hide); };
    void addMachine(Linear *machine_ , bool hide = false){ addMachine(machine_,LINEAR,hide); };
    void addMachine(LogSoftMax *machine_, bool hide = false){ addMachine(machine_,LOGSOFTMAX,hide); };
    void addMachine(Tanh *machine_, bool hide = false){ addMachine(machine_,TANH,hide); };
    void addMachine(SoftMax *machine_, bool hide = false){ addMachine(machine_,SOFTMAX,hide); };
#ifdef HAVE_SKORCH
    void addMachine(FixedLinear *machine_, bool hide = false){ addMachine(machine_,LINEAR,hide); };
    void addMachine(AdaptLinear *machine_, bool hide = false){ addMachine(machine_,LINEAR,hide); };
    void addMachine(SharedLinear *machine_, bool hide = false){ addMachine(machine_,SHAREDLINEAR,hide); };
    void addMachine(Sigmoid *machine_, bool hide = false){ addMachine(machine_,SIGMOID,hide); };
    void addMachine(SumMachine *machine_, bool hide = false){ addMachine(machine_,SUMMACHINE,hide); };
    void addMachine(BlasLinear *machine_, bool hide = false){ addMachine(machine_,BLASLINEAR,hide); };
    void addMachine(Log *machine_, bool hide = false){ addMachine(machine_,LOG,hide); };
    void addMachine(Exp *machine_, bool hide = false){ addMachine(machine_,EXP,hide); };
    void addMachine(SKBaumWelch *machine_, bool hide = false){ addMachine(machine_,BAUMWELCH,hide); };
    void addMachine(SKSoftMax *machine_, bool hide = false){ addMachine(machine_,SKSOFTMAX,hide); };
    void addMachine(ScaledLikelihood *machine, bool hide = false){ addMachine(machine, SCALEDLIKELIHOOD, hide); };
#endif
    
    /** Connect the last added machine on #machine#.
	Note that #machine# \emph{must} be in a previous layer. */
    virtual void connectOn(GradientMachine *machine_);
    
    /// Add a layer (you don't have to call that for the first layer)
    virtual void addLayer();

    /** Contruct the machine... you need to call that after adding and
	connecting all the machines. */
    virtual void build();

   //-----

    virtual void reset();
    virtual void iterInitialize();
    virtual void forward(Sequence *inputs);
    virtual void backward(Sequence *inputs, Sequence *alpha);
    virtual void setPartialBackprop(bool flag=true);
    virtual void setDataSet(DataSet *dataset_);

    virtual void frameForward(int t, real *f_inputs, real *f_outputs);

    // enables the fixing of parameters in the linear layers -- useful when using adaptation
    virtual void loadXFile(XFile *file, bool fix = false, bool hide = false); 

    virtual void saveXFile(XFile *file);

    virtual ~SKMLP();

  };
}

#endif
