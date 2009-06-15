#include "StumpMachine.h"
#include "spCore.h"

namespace Torch {

///////////////////////////////////////////////////////////////////////////
// Constructor

StumpMachine::StumpMachine() : Machine()
{
	//feature_id = -1;
	threshold = 0.0;
	direction = 0;
	m_output.resize(1);
}
//void StumpMachine::setCore(spCore* core)
//{
//	m_core = core;
//	print("you are here\n");
//}
bool StumpMachine::forward(const Tensor& input)
{
   	if(m_core == NULL)
	{
	   	Torch::error("StumpMachine::forward() no core available.");
		return false;
	}

	//DoubleTensor* t_input = (DoubleTensor*) input;

	if(m_core->process(input) == false)
	{
	   	Torch::error("StumpMachine::forward() core failed.");
		return false;
	}

	DoubleTensor *core_t_output = (DoubleTensor*) &m_core->getOutput(0);

	double feature = core_t_output->get(0);

  //  print("....feature......%f\n",feature);
	double stump_output_;

	if(direction == 1)
	{
		if(feature >= threshold) stump_output_ = 1.0;
		else stump_output_ = -1.0;
	}
	else
	{
		if(feature < threshold) stump_output_ = 1.0;
		else stump_output_ = -1.0;
	}

	m_output.set(0, stump_output_);

	return true;
}

bool StumpMachine::loadFile(File& file)
{
	int id;
	if (file.taggedRead(&id, sizeof(int), 1, "ID") != 1)
	{
		Torch::message("StumpMachine::load - failed to Read <ID> field!\n");
		return false;
	}
	if (id != getID())
	{
		Torch::message("StumpMachine::load - invalid <ID>, this is not a StumpMachine model!\n");
		return false;
	}

	if (file.taggedRead(&threshold, sizeof(float), 1, "THRESHOLD") != 1)
	{
		Torch::message("StumpMachine::load - failed to read <threshold> field!\n");
		return false;
	}


	if (file.taggedRead(&direction, sizeof(int), 1, "DIRECTION") != 1)
	{
		Torch::message("StumpMachine::load - failed to read <direction> field!\n");
		return false;
	}

	int idCore;
	if (file.taggedRead(&idCore, sizeof(int), 1, "CoreID") != 1)
	{
		Torch::message("StumpMachine::load - failed to read <CoreID> field!\n");
		return false;
	}

	print("StumpMachine::LoadFile()\n");
	print("   threshold = %g\n", threshold);
	print("   direction = %d\n", direction);
	print("   idCore = %d\n",idCore);

	m_core = spCoreManager::getInstance().get(idCore);
	if (m_core == 0)
	{
		Torch::message("StumpMachine::load - invalid <CoreID> field!\n");
		return false;
	}

	if (m_core->loadFile(file) == false)
	{
		Torch::message("StumpMachine::load - the spCore cannot be loaded!\n");
		return false;
	}

	return true;
}

bool StumpMachine::saveFile(File& file) const
{
	const int id = getID();
	if (file.taggedWrite(&id, sizeof(int), 1, "ID") != 1)
	{
		Torch::message("StumpMachine::save - failed to write <ID> field!\n");
		return false;
	}
	print("ID of the machine : %d\n",id);
	if (file.taggedWrite(&threshold, sizeof(float), 1, "THRESHOLD") != 1)
	{
		Torch::message("StumpMachine::save - failed to write <threshold> field!\n");
		return false;
	}

	if (file.taggedWrite(&direction, sizeof(int), 1, "DIRECTION") != 1)
	{
		Torch::message("StumpMachine::save - failed to write <direction> field!\n");
		return false;
	}
   	print("StumpMachine::saveFile()\n");
	print("   threshold = %g\n", threshold);
	print("   direction = %d\n", direction);

	if (m_core == NULL ||  m_core->saveFile(file) == false)
	{
		Torch::message("StumpMachine::save - the spCore cannot be saved!\n");
		return false;
	}

	return true;
}

void StumpMachine::setParams(int direction_, float threshold_)
{
	Torch::print("   StumpMachine::setParams()\n");

   	direction = direction_;
	threshold = threshold_;
}

StumpMachine::~StumpMachine()
{
}

}

