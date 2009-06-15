#include "CascadeIntegralTrainer.h"

namespace Torch
{

    extern "C" int cmp_value(const void *p1, const void *p2)
    {
        int v1 = *((int *)p1);
        int v2 = *((int *)p2);

        if (v1 > v2) return 1;
        if (v1 < v2) return -1;

        return 0;
    }
////////////////////////////////////////////////////////////////////////////////////////////
    CascadeIntegralTrainer::CascadeIntegralTrainer()
            :	m_target0(1),
            m_target1(1)
    {
        //addBOption("boosting_by_sampling",	false,	"use sampling based on weights");

        m_target0.fill(0);
        m_target1.fill(1);

        m_p_examples = 0;
        m_n_examples =0;

        m_n_cascade = 0;
        m_pos_dataset = NULL;
        m_dataset = NULL;
        m_labelledmeasure = NULL;
        verbose = getBOption("verbose");

    }

//////////////////////////////////////////////////////////////////////////////////////////////

    bool CascadeIntegralTrainer::setTrainers(FTrainer **m_ftrainer_,int n_cascade_, double *m_detection_rate_)
    {
        m_ftrainer = m_ftrainer_;
        m_n_cascade = n_cascade_;
        m_detection_rate = m_detection_rate_;

        return true;
    }
//////////////////////////////////////////////////////////////////////////////////////////////
    bool CascadeIntegralTrainer::setData(DataSet *m_pos_dataset_,DataSet *m_valid_dataset_,ImageScanDataSet *m_imagescandataset_)
    {

        m_pos_dataset  = m_pos_dataset_;
        m_valid_dataset = m_valid_dataset_;
        m_imagescandataset = m_imagescandataset_;
        return true;

    }
//////////////////////////////////////////////////////////////////////////////////////////////
    bool CascadeIntegralTrainer::train()
    {
        verbose = getBOption("verbose");
        if (verbose)
            print("CascadeTrainer::train() ...\n");

        //
        //As of now you can either train with integral image or just
        //otherwise if we pass both then it should know which plane to operate on
        // first get the count of +ve examples which have target = 1


        Tensor *tensor;
        int tp_examples = m_pos_dataset->getNoExamples();
        int tv_examples = m_valid_dataset->getNoExamples();
        //height and width can be obtained by looking at the size
        //int height, width;
        int p_count;
        int n_examples;

        tensor = m_pos_dataset->getExample(0);
        print("height %d, width %d\n",tensor->size(0),tensor->size(1));
        height = tensor->size(0);
        width = tensor->size(1);
        Tensor *example;
        long n_scanexamples;
        int n_count;
        m_threshold = new double[m_n_cascade];

//precalculating integral image
        ipIntegral *ipI = new ipIntegral();
        DoubleTensor *temptensor = new DoubleTensor();

        for (int e=0;e<tp_examples;e++)
        {

            temptensor = (DoubleTensor*)m_pos_dataset->getExample(e);

            ipI->process(*temptensor);
            temptensor = (DoubleTensor*) &ipI->getOutput(0);
            m_pos_dataset->getExample(e)->copy(temptensor);
        }


        for (int e=0;e<tv_examples;e++)
        {

            temptensor = (DoubleTensor*)m_valid_dataset->getExample(e);

            ipI->process(*temptensor);
            temptensor = (DoubleTensor*) &ipI->getOutput(0);
            m_valid_dataset->getExample(e)->copy(temptensor);
        }
////////////////////


        for (int mt = 0;mt< m_n_cascade;mt++)
        {


            m_current_cascade = mt;
            p_count = 0;
            for (int i = 0;i<tp_examples;i++)
            {
                ShortTensor *target = (ShortTensor *) m_pos_dataset->getTarget(i);
                short target_value = (*target)(0);
                if (target_value == 1)
                    p_count++;
            }

            //now fill the memory data set with equal number of +ve and -ve patterns

            if (m_dataset != NULL) delete m_dataset;

            //now create a new dataset
            n_examples = 2*p_count;
            m_dataset = 	new MemoryDataSet(n_examples, Tensor::Double, true, Tensor::Short);
            // Test the targets (rejection of samples)
            DoubleTensor reject_target(1), accept_target(1);
            reject_target.fill(-1.0);
            accept_target.fill(1.0);


            //first fill with positive patterns

            p_count=0;

            for (int i=0;i<tp_examples;i++)
            {
                ShortTensor *target = (ShortTensor *) m_pos_dataset->getTarget(i);
                short target_value = (*target)(0);
                if (target_value == 1)
                {
                    m_dataset->getExample(p_count)->resize(height, width);
                    example = m_pos_dataset->getExample(i);
                    m_dataset->getExample(p_count)->copy(example);
                    m_dataset->setTarget(p_count, &m_target1);
                    p_count++;
                }
            }


            n_scanexamples = m_imagescandataset->getNoExamples();
            if (verbose)
                print("Number of Positive patterns remaining: %d\n",p_count);
            //next fill with negative patterns

            n_count =0;
            for (int i=0;i<n_scanexamples;i++)
            {
                //have to check if the target is +1 and fill withit
                if (((DoubleTensor*)m_imagescandataset->getTarget(i))->get(0)>0.0)
                {

                    n_count++;


                }

            }

            int pnCount;
            pnCount = int(1.5*p_count);
            if (n_count< pnCount)
            {
                print("There are not enough -ve patterns for training Aborting\n");
                delete m_dataset;
                return false;
            }


            // now do the random selection


            int *randSelect = new int[pnCount];
            short *randTrack = new short[pnCount];
            for (int i=0;i<pnCount;i++)
                randTrack[i]=0;
            for (int i=0;i<pnCount;i++)
            {
                double z = THRandom_uniform(0, n_count);
                randSelect[i]=int(z);

            }

            qsort(randSelect, pnCount, sizeof(int), cmp_value);


            //check for any duplicate values
            for (int i=1;i<pnCount;i++)
            {
                if (randSelect[i] == randSelect[i-1])
                    randTrack[i]=1;
            }
            int k;
            k=0;
            for (int i=0;i<pnCount;i++)
            {
                if (randTrack[i]==0)
                {
                    randSelect[k] = randSelect[i];
                    k++;
                }
            }

            if (verbose)
                print("Number of negative patterns = %d P_count %d\n",k,p_count);

            if (k<p_count)
            {
                print("Number of -ve patterns randomly selected are not unique\n");
                return false;
            }

            //now fill the memdataset


            k=0;
            n_count=0;
            for (int i=0;i<n_scanexamples;i++)
            {
                //have to check if the target is +1 and fill withit
                if (((DoubleTensor*)m_imagescandataset->getTarget(i))->get(0)>0.0)
                {
                    if (randSelect[k]==n_count)
                    {

                        m_dataset->getExample(p_count+k)->resize(height, width);
                        example = m_imagescandataset->getExample(i);
                        m_dataset->getExample(p_count+k)->copy(example);
                        m_dataset->setTarget(p_count+k, &m_target0);
                        temptensor = (DoubleTensor*)m_dataset->getExample(p_count+k);
                        //compute integral image
                        ipI->process(*temptensor);
                        temptensor = (DoubleTensor*) &ipI->getOutput(0);
                        m_dataset->getExample(p_count+k)->copy(temptensor);

                        k++;
                        if (k==p_count)
                            break;
                    }
                    n_count++;

                }

            }
            if (verbose)
                print("number of negative patterns= %d P_count %d\n",k,p_count);

            // do a check if the memory dataset has target set
            for (int i=0;i<n_examples;i++)
            {
                ShortTensor *t_target = (ShortTensor*)m_dataset->getTarget(i);
                if ((*t_target)(0) != 0 && (*t_target)(0) != 1)
                {
                    print("The memory data set does not have proper target\n");
                    return false;
                }
            }




            //---------------finished copying data to memory------------
            if (verbose)
            {
                print("Cascade iteration number %d\n",mt);
                print("Number of examples in Memory is %d\n",m_dataset->getNoExamples());
            }
            m_ftrainer[mt]->setData(m_dataset);

            m_ftrainer[mt]->train();



            getThreshold(m_valid_dataset);
            updateDataSet(mt,m_pos_dataset,"training");
            updateDataSet(mt,m_valid_dataset,"validation");
            if (mt<m_n_cascade-1)
                updateImageScanDataSet(mt);
            // updateImageScanDataSet_check(mt);
            delete[] randSelect;

            delete [] randTrack;

            //  delete temptensor;
            //  delete ipI;
        }//end of for mt

        return true;
    }
    //////////////
    void CascadeIntegralTrainer::getThreshold(DataSet *m_data)
    {
        // Torch::print("CascadeTrainer::getThrehsold()\n");

        int tp_examples = m_data->getNoExamples();
        delete [] m_labelledmeasure;
        m_labelledmeasure = new LabelledMeasure[tp_examples];
        int count = 0;
        for (int i=0;i<tp_examples;i++)
        {
            ShortTensor *target = (ShortTensor *) m_data->getTarget(i);
            short target_value = (*target)(0);
            if (target_value == 1)
            {

                Tensor *example = m_data->getExample(i);
                m_labelledmeasure[count].measure = m_ftrainer[m_current_cascade]->forward(example);

                m_labelledmeasure[count].label = target_value;
                count++;

            }
        }



        m_threshold[m_current_cascade]= computeTH(m_labelledmeasure, count, m_detection_rate[m_current_cascade]);
        if (verbose)
            print("Threshold for stage %d is %f\n",m_current_cascade,m_threshold[m_current_cascade]);

    }
////////////////////////////////////////////////////////////////////////////////
    void CascadeIntegralTrainer::updateImageScanDataSet(int trainer_i)
    {
        long n_scanexamples = m_imagescandataset->getNoExamples();
        //next fill with negative patterns
        //double imgtarget;
        //is it possible to make it random here to get ramdom patterns
        int nc =0;
        //  Tensor *example;
        double score;
        DoubleTensor reject_target(1);
        MemoryDataSet *m_tempmemory = 	new MemoryDataSet(1, Tensor::Double, true, Tensor::Short);
        m_tempmemory->getExample(0)->resize(height, width);
        reject_target.fill(-1.0);

        ipIntegral *ipI = new ipIntegral();
        DoubleTensor *temptensor = new DoubleTensor();

        for (long i=0;i<n_scanexamples;i++)
        {
            //have to check if the target is +1 and fill withit
            if (((DoubleTensor*)m_imagescandataset->getTarget(i))->get(0)>0.0)
            {


                Tensor *example = m_imagescandataset->getExample(i);
                m_tempmemory->getExample(0)->copy(example);
                //here have to compute integral image
                temptensor =  (DoubleTensor*)m_tempmemory->getExample(0);

                ipI->process(*temptensor);

                // temptensor = (DoubleTensor*) &ipI->getOutput(0);
                m_tempmemory->getExample(0)->copy(&ipI->getOutput(0));
                example = m_tempmemory->getExample(0);
                score=m_ftrainer[trainer_i]->forward(example);
                if (score>m_threshold[m_current_cascade])
                    // set the pattern to reject
                {
                    nc++;
                }
                else

                    m_imagescandataset->setTarget(i, &reject_target);

            }
            if (i%1000001 == 0)
                if (verbose)
                    print("%d ..",i);

        }

        if (verbose)
            print("Number of -ve patterns left = %ld\n",nc);

        //  delete ipI;
        // delete temptensor;
        delete m_tempmemory;

    }

    //////////////////////
    ////////////////////////////////////////////////////////////////////////////////
    void CascadeIntegralTrainer::updateImageScanDataSet_check(int trainer_i)
    {
        long n_scanexamples = m_imagescandataset->getNoExamples();
        //next fill with negative patterns

        //is it possible to make it random here to get ramdom patterns
        int nc =0;

        DoubleTensor reject_target(1);

        reject_target.fill(-1.0);

        for (long i=0;i<n_scanexamples;i++)
        {
            //have to check if the target is +1 and fill withit
            if (((DoubleTensor*)m_imagescandataset->getTarget(i))->get(0)>0.0)
            {


                double z = THRandom_uniform(0, 1);

                if (z>0.5)
                    // set the pattern to reject
                {
                    nc++;
                }
                else

                    m_imagescandataset->setTarget(i, &reject_target);

            }

            if (i%1000001 == 0)
                if (verbose)
                    print("%d ..",i);

        }

        if (verbose)
            print("Number of -ve patterns left = %ld\n",nc);

    }

//////////////////////////////////////////////////////////////////////////////
    double CascadeIntegralTrainer::forward(Tensor *example_)
    {
        print("************ Nothing Implemented yet *******************\n");
        return 1.0;
    }

////////////////////////////////////////////////////////////////////////////////
    void CascadeIntegralTrainer::updateDataSet(int trainer_i, DataSet *mdata_,const char *string_text)
    {

        //update the positive training and/or validation dataset

        //get the target value and only process it is = 1
        int tp_examples = mdata_->getNoExamples();
        int count = 0;
        for (int i=0;i<tp_examples;i++)
        {
            ShortTensor *target = (ShortTensor *) mdata_->getTarget(i);
            short target_value = (*target)(0);
            if (target_value == 1)
            {

                Tensor *example = mdata_->getExample(i);
                if (m_ftrainer[m_current_cascade]->forward(example)<m_threshold[m_current_cascade])
                {

                    mdata_->setTarget(i, &m_target0);
                    count++;
                }

            }
        }
        if (verbose)
            print("Number of %s examples rejected  = %d\n",string_text,count);

    }
////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
    CascadeIntegralTrainer::~CascadeIntegralTrainer()
    {

        delete[] m_threshold;
        delete m_dataset;
        delete [] m_labelledmeasure;
    }

}
