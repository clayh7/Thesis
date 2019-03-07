#include "Engine/Threads/JobSystem.hpp"

#include <thread>


//-------------------------------------------------------------------------------------------------
void JobSystemThreadEntry( void * )
{
	JobConsumer consumer;
	consumer.AddCategory( eJobCategory_GENERIC_SLOW );
	consumer.AddCategory( eJobCategory_GENERIC );
	while( g_JobSystem->IsRunning() )
	{
		consumer.ConsumeAll( );
		std::this_thread::yield( );
	}
	consumer.ConsumeAll( );
}


//-------------------------------------------------------------------------------------------------
JobConsumer::JobConsumer( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
void JobConsumer::AddCategory( eJobCategory const & category )
{
	m_categories.push_back( category );
}


//-------------------------------------------------------------------------------------------------
void JobConsumer::ConsumeAll( )
{
	while( Consume( ) );
}


//-------------------------------------------------------------------------------------------------
bool JobConsumer::Consume( )
{
	for( eJobCategory const & category : m_categories )
	{
		Job * job;
		BQueue<Job*> * queue = g_JobSystem->GetJobQueue( category );
		if( queue->PopFront(&job) )
		{
			job->Work( );
			g_JobSystem->Finish( job );
			return true;
		}
	}
	return false;
}


//-------------------------------------------------------------------------------------------------
JobSystem * g_JobSystem = nullptr;


//-------------------------------------------------------------------------------------------------
JobSystem::JobSystem( )
	: m_jobMemoryPool( MAX_JOBS )
	, m_isRunning( true )
{
	for( size_t jobCategoryIndex = 0; jobCategoryIndex < eJobCategory_COUNT; ++jobCategoryIndex )
	{
		m_jobQueue.push_back( new BQueue<Job*>( ) );
	}
}


//-------------------------------------------------------------------------------------------------
JobSystem::~JobSystem( )
{
	m_isRunning = false;

	for( size_t threadIndex = 0; threadIndex < m_threads.size( ); ++threadIndex )
	{
		m_threads[threadIndex].Join( );
	}

	m_criticalSection.Lock( );
	m_jobMemoryPool.Destroy( );
	m_criticalSection.Unlock( );

	for( size_t jobCategoryIndex = 0; jobCategoryIndex < eJobCategory_COUNT; ++jobCategoryIndex )
	{
		delete m_jobQueue[jobCategoryIndex];
		m_jobQueue[jobCategoryIndex] = nullptr;
	}
	m_jobQueue.clear( );
}


//-------------------------------------------------------------------------------------------------
void JobSystem::Startup( int numOfThreads )
{
	if( numOfThreads < 0 )
	{
		numOfThreads += GetCoreCount( );
	}

	if( numOfThreads < 0 )
	{
		numOfThreads = 1;
	}

	for( int threadIndex = 0; threadIndex < numOfThreads; ++threadIndex )
	{
		m_threads.push_back( Thread( JobSystemThreadEntry ) );
	}
}


//-------------------------------------------------------------------------------------------------
Job * JobSystem::JobCreate( eJobCategory const & category, JobCallback * jobFunc )
{
	m_criticalSection.Lock( );
	Job * newJob = m_jobMemoryPool.Alloc( );
	m_criticalSection.Unlock( );

	newJob->m_category = category;
	newJob->m_jobFunc = jobFunc;
	++newJob->m_refCount;
	return newJob;
}


//-------------------------------------------------------------------------------------------------
void JobSystem::JobDispatch( Job * job )
{
	++job->m_refCount;
	m_jobQueue[job->m_category]->PushBack( job );
}


//-------------------------------------------------------------------------------------------------
void JobSystem::JobDetach( Job * job )
{
	--job->m_refCount;
	if( job->m_refCount == 0 )
	{
		m_criticalSection.Lock( );
		m_jobMemoryPool.Delete( job );
		m_criticalSection.Unlock( );
	}
}


//-------------------------------------------------------------------------------------------------
void JobSystem::JobJoin( Job * job )
{
	while( job->m_refCount == 2 );
	--job->m_refCount;
	if( job->m_refCount == 0 )
	{
		m_criticalSection.Lock( );
		m_jobMemoryPool.Delete( job );
		m_criticalSection.Unlock( );
	}
	else
	{
		ERROR_AND_DIE( "Job still has a reference" );
	}
}


//-------------------------------------------------------------------------------------------------
void JobSystem::Finish( Job * job )
{
	--job->m_refCount;
	if( job->m_refCount == 0 )
	{
		m_criticalSection.Lock( );
		m_jobMemoryPool.Delete( job );
		m_criticalSection.Unlock( );
	}
}


//-------------------------------------------------------------------------------------------------
size_t JobSystem::GetCoreCount() const
{
	return (size_t) std::thread::hardware_concurrency( );
}


//-------------------------------------------------------------------------------------------------
BQueue<Job*> * JobSystem::GetJobQueue( eJobCategory const & category ) const
{
	return m_jobQueue[(size_t) category];
}


//-------------------------------------------------------------------------------------------------
bool JobSystem::IsRunning() const
{
	return m_isRunning;
}
