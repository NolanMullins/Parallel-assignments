#include "data.h"

// calculate the dot product between two vectors
float dotProduct(float x1, float y1, float x2, float y2)
{
	return (x1 * x2 + y1 * y2);
}

int ballCollision(float4 balli, float4 ballj)
{
	__local float distance;
	__local float radiiSum;

	// Pythagorean distance
	distance = sqrt(pow((balli.s0 - ballj.s0), 2) + pow((balli.s1 - ballj.s1), 2));
	radiiSum = RADIUS + RADIUS;

	if (balli.s0 == 142 && balli.s0 == 30 && balli.s0 == 30 && ballj.s1 == 30)
		printf("dist: %.1f\n", distance);

	// if the sum of the two radii is less than the distance
	// between the balls then they collide, otherwise they
	// do not collide
	if (distance < radiiSum)
		return (COLLIDE);
	else
		return (NOCOLLIDE);
}

float2 resolveCollision(float i, float4 balli, float j, float4 ballj)
{
	__local float rvx, rvy;
	__local float nx, ny;
	__local float distance;
	__local float vnormal;
	__local float impulse;
 	__local	float ix, iy;

	__local float2 vel;
	vel.s0 = 0;
	vel.s1 = 0;

	// calculate relative velocity
	rvx = ballj.s2 - balli.s2;
	rvy = ballj.s3 - balli.s3;

	// calculate collision normal
	nx = ballj.s0 - balli.s0;
	ny = ballj.s1 - balli.s1;

	// Pythagorean distance
	distance = sqrt(pow((ballj.s0 - balli.s0), 2) + pow((ballj.s1 - balli.s1), 2));
	if (distance == 0)
		return vel;

	nx = nx / distance;
	ny = ny / distance;

	// Calculate relative velocity in terms of the normal direction
	vnormal = dotProduct(rvx, rvy, nx, ny);

	// Do not resolve if velocities are separating
	if (vnormal > 0)
		return vel;

	// Calculate impulse scalar
	impulse = -(1 + RESTITUTION) * vnormal;
	impulse /= ((1 / MASS) + (1 / MASS));

	// Apply impulse
	int dir = 1;
	if (i < j)
		dir = -1;
	
	vel.s0 = balli.s2 + dir * ((1 / MASS) * impulse);
	vel.s1 = balli.s3 + dir * ((1 / MASS) * impulse);

	return vel;
}

__kernel void phys(__global float4 *data,
				   __local float2 *local_result, __global float4 *group_result)
{
	float4 ball;
	float2 local_vel;
	float2 vel;
	uint global_addr, local_id;
	int group_id, a;

	global_addr = get_global_id(0);
	group_id = get_group_id(0);
	local_id = get_local_id(0);
	ball = data[group_id];
    
	vel.s0 = 0;
	vel.s1 = 0;

	for (a = 0; a < POPSIZE; a++)
		if (a != group_id)
			if (ballCollision(ball, data[a]) == COLLIDE)
				vel = resolveCollision(group_id, ball, a, data[a]);

	/*local_result[local_id] = local_vel;

	barrier(CLK_LOCAL_MEM_FENCE);
	if (local_id == 0)
	{
		for (a = 0; a < 4; a++) 
		{
			if (fabs(local_result[a].s0) >= 0.01)
			{	
				vel = local_result[a];
			}
		}
		group_result[get_group_id(0)] = vel;
	}*/

	barrier(CLK_GLOBAL_MEM_FENCE);

	// update velocity for each ball
	if (fabs(vel.s0) >= 0.01)
		ball.s2 = vel.s0;
	if (fabs(vel.s1) >= 0.01)
		ball.s3 = vel.s1;

	// enforce maximum velocity of 2.0 in each axis
	// done to make it easier to see collisions
	if (ball.s2 > 2.0)
		ball.s2 = 2.0;
	if (ball.s3 > 2.0)
		ball.s3 = 2.0;
    
	// update position for each ball
	ball.s0 += ball.s2;
	ball.s1 += ball.s3;
    
	// if ball moves off the screen then reverse velocity so it bounces
	// back onto the screen, and move it onto the screen
	if (ball.s0 > (SCREENSIZE - 1))
	{
		ball.s2 *= -1.0;
		ball.s0 = SCREENSIZE - 1.5;
	}
	if (ball.s0 < 0.0)
	{
		ball.s2 *= -1.0;
		ball.s0 = 0.5;
	}
	if (ball.s1 > (SCREENSIZE - 1))
	{
		ball.s3 *= -1.0;
		ball.s1 = SCREENSIZE - 1.5;
	}
	if (ball.s1 < 0.0)
	{
		ball.s3 *= -1.0;
		ball.s1 = 0.5;
	}

	group_result[group_id] = ball;
}
