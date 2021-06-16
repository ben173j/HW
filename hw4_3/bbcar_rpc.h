#ifndef BBCAR_RPC_H
#define BBCAR_RPC_H

#include "bbcar.h"
#include "mbed_rpc.h"

void RPC_stop(Arguments *in, Reply *out);
void RPC_goStraight(Arguments *in, Reply *out);
void RPC_turn(Arguments *in, Reply *out);
void RPC_receive_LENGTH(Arguments *in, Reply *out);
void RPC_COMBINATION(Arguments* in, Reply* out);
void RPC_apriltag(Arguments* in, Reply* out);

#endif
