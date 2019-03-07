#pragma once

#include <atomic>
#include <vector>
#include "Engine/Threads/BQueue.hpp"
#include "Engine/Threads/Thread.hpp"
#include "Engine/Memory/ObjectPool.hpp"
#include "Engine/Threads/Job.hpp"


//-------------------------------------------------------------------------------------------------
void JobSystemThreadEntry( void * );


//-------------------------------------------------------------------------------------------------
class JobConsumer
{
private:
	std::vector<eJobCategory> m_categories;

public:
	JobConsumer( );

	void AddCategory( eJobCategory const & category );
	void ConsumeAll( );
	bool Consume( );
};


//-------------------------------------------------------------------------------------------------
class JobSystem;


//-------------------------------------------------------------------------------------------------
extern JobSystem * g_JobSystem;


//-------------------------------------------------------------------------------------------------
class JobSystem
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static const int MAX_JOBS = 1024;

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	std::vector<BQueue<Job*>*> m_jobQueue;
	std::vector<Thread> m_threads;
	ObjectPool<Job> m_jobMemoryPool; //Figure out how big to initialize it
	bool m_isRunning;
	CriticalSection m_criticalSection;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	JobSystem( );
	~JobSystem( );

	void Startup( int numOfThreads );
	Job * JobCreate( eJobCategory const & category, JobCallback * jobFunc );
	void JobDispatch( Job * job );
	void JobDetach( Job * job );
	void JobJoin( Job * job );
	void Finish( Job * job );

	size_t GetCoreCount( ) const;
	BQueue<Job*> * GetJobQueue( eJobCategory const & category ) const;
	bool IsRunning( ) const;
};