/*
 * roby.h
 *
 *  Created on: Sep 19, 2018
 *      Author: sasha
 */

#ifndef ROBY_H_
#define ROBY_H_

#define  A_D            0x01
#define  A_S            0x02
#define  A_X_FORWARD    0x04
#define  A_X_BACK       0x08
#define  A_Z_BACK       0x10
#define  A_Z_FORWARD    0x20
#define  A_Y_BACK       0x40
#define  A_Y_FORWARD    0x80

#define  B_X            0x01
#define  B_Y            0x02
#define  B_Z            0x04
#define  B_W_END        0x08
#define  B_W_BEGIN      0x10
#define  B_Z_BEGIN      0x20
#define  B_Y_BEGIN      0x40
#define  B_X_BEGIN      0x80

#define  C_F_END        0x04
#define  C_F_BEGIN      0x08
#define  C_W_FORWARD    0x10
#define  C_W_BACK       0x20
#define  C_F_FORWARD    0x40
#define  C_F_BACK       0x80


#define  X_MIN          0
#define  X_MAX          1024
#define  Y_MIN          0
#define  Y_MAX          1024
#define  Z_MIN          0
#define  Z_MAX          1024
#define  W_MIN          0
#define  W_MAX          100
#define  F_MIN          0
#define  F_MAX          50
#define  S_MIN          0
#define  S_MAX          1
#define  D_MIN          0
#define  D_MAX          1

#define W_END           0x08
#define W_BEGIN         0x10
#define F_END           0x20
#define F_BEGIN         0x40

#endif /* ROBY_H_ */
