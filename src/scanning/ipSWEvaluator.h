#ifndef _TORCHVISION_SCANNING_IP_SW_EVALUATOR_H_
#define _TORCHVISION_SCANNING_IP_SW_EVALUATOR_H_

#include "ipCore.h"		// <ipSWEvaluator> is an <ipCore>

namespace Torch
{
        class Classifier;

	/////////////////////////////////////////////////////////////////////////
	// Torch::ipSWEvaluator:
	//	- use some 2-class classifier to check if some sub-window contains a pattern
	//	- the actual processing is done in <setRegion> for efficiency reasons!
	//
	//      - MULTISCALE approach (the input size is larger than the model's):
        //              - rescale the input tensor to the buffer, presuming
        //                      the input tensor has an integral image like format
        //      - PYRAMID approach (the input size is the same as the model's):
        //              - crop the input tensor to the buffer (actually just copy the values)
        //
	// TODO: doxygen header!
	/////////////////////////////////////////////////////////////////////////

	class ipSWEvaluator : public ipCore
	{
	public:

		// Constructor
		ipSWEvaluator();

		// Destructor
		virtual ~ipSWEvaluator();

		// Set the classifier to load from some file
		bool                    setClassifier(const char* filename);

		/// Change the region of the input tensor to process
		void			setRegion(const TensorRegion& region);

		/////////////////////////////////////////////////////////////////
		// Access functions

		// Get the result - the sub-window contains the pattern?!
		bool			isPattern() const;
		//	... get the model confidence of this
		double			getConfidence() const;

		// Get the model size
		int		        getModelWidth() const;
		int	        	getModelHeight() const;

		const Classifier&	getClassifier() const;
		Classifier&		getClassifier();

		/////////////////////////////////////////////////////////////////

	protected:

                /////////////////////////////////////////////////////////////////

                /// Check if the input tensor has the right dimensions and type - overriden
		virtual bool		checkInput(const Tensor& input) const;

		/// Allocate (if needed) the output tensors given the input tensor dimensions - overriden
		virtual bool		allocateOutput(const Tensor& input);

		/// Process some input tensor (the input is checked, the outputs are allocated) - overriden
		/// The classifier should be loaded first!
		virtual bool		processInput(const Tensor& input);

		/////////////////////////////////////////////////////////////////

        private:

                /////////////////////////////////////////////////////////////////
		// Attributes

		// Machine used for deciding if some sub-window contains a pattern or not
		Classifier*             m_classifier;

		// Keep a copy of the input tensor (to pass to setSubWindow)
                const Tensor*           m_input_copy;
	};
}

#endif
