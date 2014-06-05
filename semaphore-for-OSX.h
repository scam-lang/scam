
/*
 *  Main Author : John C. Lusth
 *  Added Header : Jeffrey Robinson
 *  Last Edit : May 4, 2014
 *
 * An interface common to the implementation of Semaphores across
 *   all supported platforms.
 *
 */


#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <cstdlib>
#include <cstdio>

class Semaphore
{
public:
    virtual void signal  (void) = 0;
    virtual void wait    (void) = 0;

    virtual int  get_val (void) = 0; /* Mostly unsupported! */
    virtual void set_val (int)  = 0;

protected:
private:
};

/* A "Mutex" is just a special form of Semaphore. */
typedef Semaphore Mutex;

#ifdef __APPLE__

#include <mach/mach_traps.h>
#include <mach/semaphore.h>
#include <mach/task.h>
#include <mach/mach.h>

/**
 * MacOS X Implementation of Semaphore...
 *
 *   MacOS X cannot use the same implementation as Linux, because
 *     OS X does not support unnamed semaphores using the POSIX-defined
 *       API.
 *
 *   The only way to create unnamed semaphores in OS X is to use
 *     low-level Mach kernel API calls. And the operations supported
 *       at such a low-level are very limited by comparison.
 *
**/
class Semaphore_OSX : public Semaphore
{
public:
  Semaphore_OSX (int init_val) {
    semaphore_create (mach_task_self (), &sem_, SYNC_POLICY_FIFO, init_val);
  }

  inline void signal (void) { semaphore_signal (sem_); }
  inline void wait   (void) { semaphore_wait   (sem_); }

  inline void set_val (int val) { 
    semaphore_destroy (mach_task_self (),  sem_);
    semaphore_create  (mach_task_self (), &sem_, SYNC_POLICY_FIFO, val);
  }

  inline int get_val (void) {
    return 0xFFFFFFFF; /* Unsupported! */
  }

protected:
private:
  semaphore_t sem_;
};

#endif /* __APPLE_ */


#ifdef __linux__

#include <semaphore.h>

/**
 * Linux implementation of Semaphore.
 *
 *   This is the easiest platform, by far, to implement
 *     unnamed semaphores in. No detailed explanation
 *       should be necessary.
 *
**/
class Semaphore_Linux : public Semaphore
{
public:
  Semaphore_Linux (int init_val) {
    sem_init (&sem_, 0, init_val);
  }

  inline void signal (void) { sem_post (&sem_); }
  inline void wait   (void) { sem_wait (&sem_); }

  inline void set_val (int val) {
    sem_destroy (&sem_);
    sem_init    (&sem_, 0, val);
  }

  inline int get_val (void) {
    return 0xFFFFFFFF; /* Unsupported! */
  }

protected:
private:
  sem_t sem_;
};

#endif /* __linux__ */

#ifdef __WIN32__

/**
 * This is where I would implement Semaphores using the Win32 API,
 *   if I actually cared about Windows...
 *
 *   I would have to rewrite "thread_imp.h" to support threading
 *     on Win32, so supporting Semaphores on Win32 is really
 *       pointless for this assignment.
 *
**/
class Semaphore_Win32 : public Semaphore
{
public:
protected:
private:
};

#endif /* __WIN32__ */


/**
 * Create a Semaphore with initial value equal to init_val.
 *
 *   This function takes care of the platform-specific
 *     implementation transparently, so that the host
 *       application will run on all platforms without
 *         knowing anything about the actual implementation.
 *
**/
static
Semaphore*
CreateSemaphore (int init_val)
{
#ifdef __APPLE__
  return new Semaphore_OSX   (init_val);
#elif defined (__linux__)
  return new Semaphore_Linux (init_val);
#elif defined (__WIN32__)
  return new Semaphore_Win32 (init_val);
#else
  printf ("Semaphores are unsupported on your platform!\n");
  abort  ();
#endif
  return NULL;
}

/**
 * Creates a "Mutex", which is really nothing more than
 *   a Semaphore with an initial value of 1.
 *
 *   It is important to initialize to 1 so that 1 thread,
 *     and only 1 thread, may enter the critical section
 *       guarded by the Mutex at a time.
 *
**/
static
Mutex*
CreateMutex (void)
{
  return CreateSemaphore (1);
}



#endif /* __SEMAPHORE_H__ */
