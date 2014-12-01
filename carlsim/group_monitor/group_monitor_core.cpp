#include <group_monitor_core.h>

#include <snn.h>				// CARLsim private implementation
#include <snn_definitions.h>	// KERNEL_ERROR, KERNEL_INFO, ...

#include <algorithm>			// std::sort



// we aren't using namespace std so pay attention!
GroupMonitorCore::GroupMonitorCore(CpuSNN* snn, int monitorId, int grpId) {
	snn_ = snn;
	grpId_= grpId;
	monitorId_ = monitorId;

	groupFileId_ = NULL;
	recordSet_ = false;
	grpMonLastUpdated_ = 0;

	persistentData_ = false;

	needToWriteFileHeader_ = true;
	groupFileSignature_ = 206661989;
	groupFileVersion_ = 0.2f;

	// defer all unsafe operations to init function
	init();
}

void GroupMonitorCore::init() {
	nNeurons_ = snn_->getGroupNumNeurons(grpId_);
	assert(nNeurons_> 0);

	clear();

	// use KERNEL_{ERROR|WARNING|etc} typesetting (const FILE*)
	fpInf_ = snn_->getLogFpInf();
	fpErr_ = snn_->getLogFpErr();
	fpDeb_ = snn_->getLogFpDeb();
	fpLog_ = snn_->getLogFpLog();
}

GroupMonitorCore::~GroupMonitorCore() {
	if (groupFileId_ != NULL) {
		fclose(groupFileId_);
		groupFileId_ = NULL;
	}
}

// +++++ PUBLIC METHODS: +++++++++++++++++++++++++++++++++++++++++++++++//

void GroupMonitorCore::clear() {
	assert(!isRecording());
	recordSet_ = false;
	startTime_ = -1;
	startTimeLast_ = -1;
	stopTime_ = -1;
	accumTime_ = 0;
	totalTime_ = -1;

	timeVector_.clear();
	dataVector_.clear();
}

void GroupMonitorCore::print() {
	assert(!isRecording());
}

void GroupMonitorCore::pushData(unsigned int time, float data) {
	assert(isRecording());

	timeVector_.push_back(time);
	dataVector_.push_back(data);
}

void GroupMonitorCore::startRecording() {
	assert(!isRecording());

	if (!persistentData_) {
		// if persistent mode is off (default behavior), automatically call clear() here
		clear();
	}

	// call updateGroupMonitor to make sure group file and data vector are up-to-date
	// Caution: must be called before recordSet_ is set to true!
	snn_->updateGroupMonitor(grpId_);

	recordSet_ = true;
	unsigned int currentTime = snn_->getSimTimeSec()*1000+snn_->getSimTimeMs();

	if (persistentData_) {
		// persistent mode on: accumulate all times
		// change start time only if this is the first time running it
		startTime_ = (startTime_<0) ? currentTime : startTime_;
		startTimeLast_ = currentTime;
		accumTime_ = (totalTime_>0) ? totalTime_ : 0;
	}
	else {
		// persistent mode off: we only care about the last probe
		startTime_ = currentTime;
		startTimeLast_ = currentTime;
		accumTime_ = 0;
	}
}

void GroupMonitorCore::stopRecording() {
	assert(isRecording());
	assert(startTime_>-1 && startTimeLast_>-1 && accumTime_>-1);

	// call updateSpikeMonitor to make sure spike file and spike vector are up-to-date
	// Caution: must be called before recordSet_ is set to false!
	snn_->updateSpikeMonitor(grpId_);

	recordSet_ = false;
	stopTime_ = snn_->getSimTimeSec()*1000+snn_->getSimTimeMs();

	// total time is the amount of time of the last probe plus all accumulated time from previous probes
	totalTime_ = stopTime_-startTimeLast_ + accumTime_;
	assert(totalTime_>=0);
}

void GroupMonitorCore::setGroupFileId(FILE* groupFileId) {
	assert(!isRecording());

	// \TODO consider the case where this function is called more than once
	if (groupFileId_ != NULL)
		KERNEL_ERROR("GroupMonitorCore: setGroupFileId() has already been called.");

	groupFileId_ = groupFileId;

	if (groupFileId_ == NULL)
		needToWriteFileHeader_ = false;
	else {
		// for now: file pointer has changed, so we need to write header (again)
		needToWriteFileHeader_ = true;
		writeGroupFileHeader();
	}
}

// write the header section of the spike file
// this should be done once per file, and should be the very first entries in the file
void GroupMonitorCore::writeGroupFileHeader() {
	if (!needToWriteFileHeader_)
		return;

	// write file signature
	if (!fwrite(&groupFileSignature_, sizeof(int), 1, groupFileId_))
		KERNEL_ERROR("GroupMonitorCore: writeSpikeFileHeader has fwrite error");

	// write version number
	if (!fwrite(&groupFileVersion_, sizeof(float), 1, groupFileId_))
		KERNEL_ERROR("GroupMonitorCore: writeGroupFileHeader has fwrite error");

	// write grid dimensions
	Grid3D grid = snn_->getGroupGrid3D(grpId_);
	int tmpInt = grid.x;
	if (!fwrite(&tmpInt,sizeof(int), 1, groupFileId_))
		KERNEL_ERROR("GroupMonitorCore: writeGroupFileHeader has fwrite error");

	tmpInt = grid.y;
	if (!fwrite(&tmpInt,sizeof(int),1,groupFileId_))
		KERNEL_ERROR("GroupMonitorCore: writeGroupFileHeader has fwrite error");

	tmpInt = grid.z;
	if (!fwrite(&tmpInt,sizeof(int),1,groupFileId_))
		KERNEL_ERROR("GroupMonitorCore: writeGroupFileHeader has fwrite error");


	needToWriteFileHeader_ = false;
}
