
//---------------------------------------------------------------------
// Number of iterations to send the double array when measuring the
// throughput of the connection
//---------------------------------------------------------------------
#define NUM_THROUGHPUT_ITERATIONS 10000

//---------------------------------------------------------------------
// Size of the double array we send each iteration
//---------------------------------------------------------------------
#define DOUBLES_PER_ITERATION 200000

//---------------------------------------------------------------------
// The number of iterations we send data back and forth when testing
// the latency of the socket
//---------------------------------------------------------------------
#define NUM_LATENCY_ITERATIONS 300000

//---------------------------------------------------------------------
// The port used when testing TCP connection performance
//---------------------------------------------------------------------
#define TEST_TCP_PORT 50051
#define TEST_TCP_PORT_STR "50051"

//---------------------------------------------------------------------
// The port we use when testing hypervision sockets performance
// Note: this port cannot be changed without changing the GUID used
//       when registering the service with windows.  The windows
//       client will also pass a GUID with the port encoded in it
//       See the readme for more information.
//---------------------------------------------------------------------
#define TEST_VSOCK_PORT 50053