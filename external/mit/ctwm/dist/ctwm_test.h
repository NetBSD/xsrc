/**
 * Special magic for use in tests.  If you're not one of our tests, you
 * probably shouldn't be including this file.  Or even looking at it.
 * Avert your eyes!
 */

#ifndef _CTWM_CTWM_TEST_H
#define _CTWM_CTWM_TEST_H

// extern's for our magic flag and callback connections
extern int (*ctwm_test_postparse)(void);
extern bool ctwm_test;


// Provide a macro for hooking up callback
#define TEST_POSTPARSE(cb) do { \
               ctwm_test = true; \
               ctwm_test_postparse = (cb); \
        } while(0)


#endif // _CTWM_CTWM_TEST_H
