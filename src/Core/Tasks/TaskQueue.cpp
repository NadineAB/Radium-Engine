#include <Core/Tasks/TaskQueue.hpp>
#include <Core/Tasks/Task.hpp>

namespace Ra
{
    namespace Core
    {

        TaskQueue::TaskQueue( int numThreads )
            : m_processingTasks( 0 ), m_shuttingDown(false)
        {
            m_workerThreads.reserve(numThreads);
            for  (uint i = 0 ; i < numThreads; ++i)
            {
                m_workerThreads.emplace_back(std::thread( &TaskQueue::runThread, this, i ));
            }
        }

        TaskQueue::~TaskQueue()
        {
            flushTaskQueue();
            m_shuttingDown = true;
            m_threadNotifier.notify_all();
            for (auto& t :  m_workerThreads)
            {
                t.join();
            }
        }

        TaskQueue::TaskId TaskQueue::registerTask( Task* task )
        {
            m_tasks.emplace_back( std::unique_ptr<Task> ( task ) );
            m_dependencies.push_back( std::vector<TaskId>() );
            m_remainingDependencies.push_back( 0 );
            TimerData tdata;
            tdata.taskName = task->getName();
            m_timerData.push_back( tdata );

            CORE_ASSERT( m_tasks.size() == m_dependencies.size(), "Inconsistent task list" );
            CORE_ASSERT( m_tasks.size() == m_remainingDependencies.size(), "Inconsistent task list" );
            CORE_ASSERT( m_tasks.size() == m_timerData.size(), "Inconsistent task list" );
            return TaskId( m_tasks.size() - 1 );
        }

        void TaskQueue::addDependency( TaskQueue::TaskId predecessor, TaskQueue::TaskId successor )
        {
            CORE_ASSERT( ( predecessor != InvalidTaskId ) && ( predecessor < m_tasks.size() ), "Invalid predecessor task" );
            CORE_ASSERT( ( successor != InvalidTaskId ) && ( successor < m_tasks.size() ), "Invalid successor task" );
            CORE_ASSERT( predecessor != successor, "Cannot add self-dependency" );

            // Todo : check for cycles.

            m_dependencies[predecessor].push_back( successor );
            ++m_remainingDependencies[successor];
        }

        void TaskQueue::queueTask( TaskQueue::TaskId task )
        {
            CORE_ASSERT( m_remainingDependencies[task] == 0, " Task has unsatisfied dependencies" );
            m_taskQueue.push_front( task );
        }

        void TaskQueue::processTaskQueue()
        {
            // Enqueue all tasks with no dependencies.
            for (uint t = 0; t < m_tasks.size(); ++t)
            {
                if (m_remainingDependencies[t] == 0)
                {
                    // TODO : notify in queueTask ?.
                    queueTask(t);
                }
            }

            // Wake up all threads.
            m_threadNotifier.notify_all();
            bool isFinished = false;
            while ( !isFinished )
            {
                // TODO : use a notifier for task queue empty.
                m_taskQueueMutex.lock();
                isFinished = (m_taskQueue.empty() && m_processingTasks == 0 );
                m_taskQueueMutex.unlock();
                if (!isFinished)
                {
                    std::this_thread::yield();
                }
            }
        }

        const std::vector<TaskQueue::TimerData>& TaskQueue::getTimerData()
        {
            return m_timerData;
        }

        void TaskQueue::flushTaskQueue()
        {
            CORE_ASSERT( m_processingTasks == 0, "You have tasks still in process" );
            CORE_ASSERT( m_taskQueue.empty(), " You have unprocessed tasks " );
            m_tasks.clear();
            m_dependencies.clear();
            m_timerData.clear();
            m_remainingDependencies.clear();
        }

        void TaskQueue::runThread(uint id)
        {
            while (true)
            {
                TaskId task = InvalidTaskId;

                // Acquire mutex.
                {
                    std::unique_lock<std::mutex> lock(m_taskQueueMutex);

                    // Wait for a new task
                    // TODO : use the second form
                    while ( !m_shuttingDown && m_taskQueue.empty() )
                    {
                        m_threadNotifier.wait(lock);
                    }
                    // If the task queue is shutting down we quit, releasing
                    // the lock.
                    if (m_shuttingDown) { return; }

                    // If we are here it means we got a task
                    task = m_taskQueue.back();
                    m_taskQueue.pop_back();
                    ++m_processingTasks;
                    CORE_ASSERT( task != InvalidTaskId && task < m_tasks.size(), "Invalid task" );
                }
                // Release mutex.

                // Run task
                m_timerData[task].start = Timer::Clock::now();
                m_tasks[task]->process();
                m_timerData[task].end = Timer::Clock::now();

                // Critical section : mark task as finished and en-queue dependencies.
                uint newTasks = 0;
                {
                    std::unique_lock<std::mutex> lock(m_taskQueueMutex);
                    for ( auto t : m_dependencies[task] )
                    {
                        uint& nDepends = m_remainingDependencies[t];
                        CORE_ASSERT( nDepends > 0, "Inconsistency in dependencies" );
                        --nDepends;
                        if ( nDepends == 0 )
                        {
                            queueTask( t );
                            ++newTasks;
                        }
                        // Easy optimization : grab one of the new task and process it immediately.
                    }
                    --m_processingTasks;
                }
                if (newTasks > 0)
                {
                    m_threadNotifier.notify_one();
                }
            } // End of while(true)
        }
    }
}
