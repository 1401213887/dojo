#include "BackgroundWorker.h"

#include "Platform.h"

using namespace Dojo;

BackgroundWorker::BackgroundWorker(bool async) :
	mRunning(false),
	mCompletedQueue(make_unique<SPSCQueue<AsyncJob>>()),
	mQueue(make_unique<SPSCQueue<AsyncJob>>()),
	isAsync(async),
	mAvailableTasksSemaphore(0) {

	if (isAsync) {
		startAsync();
	}
	else {
		mRunning = true;
	}
}

BackgroundWorker::~BackgroundWorker() {
	if (mRunning) {
		stop();
	}
}

AsyncJob BackgroundWorker::_waitForNextTask() {
	AsyncJob job;

	if (isAsync) {
		//TODO try work-stealing here
	// 	if(!mQueue->try_dequeue(job)) {
	//      teek jerbs
	// 	}

		do {
			mAvailableTasksSemaphore.wait();

			//the thread was killed while sleeping, return nothing
			if (!mRunning) {
				return{};
			}

		} while (!mQueue->try_dequeue(job)); //it's not sure that a job will be there, condition_variable has spurious wakeups

		return job;
	}
	else if(mQueue->try_dequeue(job)) { //just try to get one and return
		return job;
	}

	return{};
}

bool BackgroundWorker::runNextTask() {
	if (auto job = _waitForNextTask()) {
		auto& status = *job.mStatus;
		status = AsyncJob::Status::Running;
		job.task();

		if (job.callback) {
			status = AsyncJob::Status::Callback;
			mCompletedQueue->enqueue(std::move(job));
		}
		return true;
	}
	return false;
}

void BackgroundWorker::startAsync() {
	DEBUG_ASSERT(mRunning == false && !mThread.joinable(), "Already running");

	mRunning = true;
	mThread = std::thread([this] {
		while(mRunning) {
			runNextTask();

			std::this_thread::yield(); //go easy on the CPU, these tasks aren't priority anyway
		}
	});
}

void BackgroundWorker::queueJob(AsyncJob&& job) {
	mQueue->enqueue(std::move(job));
	mAvailableTasksSemaphore.notifyOne();
}

void BackgroundWorker::stop() {
	if (mRunning) {
		mRunning = false;

		//wake up the thread so it can kill itself (muahah)
		mAvailableTasksSemaphore.notifyOne();

		mThread.join();
	}
}

bool BackgroundWorker::_runOneCallback() {
	AsyncJob job;
	if(mCompletedQueue->try_dequeue(job)){
		job.callback();
		return true;
	}
	return false;
}

bool BackgroundWorker::_runAllCallbacks() {
	int runTasks = 0;
	while(true) {
		if(_runOneCallback()) {
			++runTasks;
		}
		else {
			break;
		}
	}
	return runTasks > 0;
}

void BackgroundWorker::sync() {
	DEBUG_ASSERT(isAsync, "TODO implement for sync queues");
	if (mRunning) {
		_runAllCallbacks(); //run all callbacks because they might have tasks which queue new jobs here

		//repeat until all the jobs have been pushed
		do {
			std::atomic<bool> done = false;
			queueJob(AsyncJob{ [this, &done]() {
				done = true;
				return true;
			},
			[] {} });
			while(!done) {
				std::this_thread::yield();
			}
		} while (_runOneCallback());
	}
}
