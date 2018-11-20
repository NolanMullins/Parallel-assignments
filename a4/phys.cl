#include "data.h"

// calculate the dot product between two vectors
float dotProduct(float x1, float y1, float x2, float y2)
{
	return (x1 * x2 + y1 * y2);
}

int ballCollision(float4 balli, float4 ballj)
{
	float distance;
	float radiiSum;

	// Pythagorean distance
	distance = sqrt(pow((balli.s0 - ballj.s0), 2) + pow((balli.s1 - ballj.s1), 2));
	radiiSum = RADIUS + RADIUS;

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
	float rvx, rvy;
	float nx, ny;
	float distance;
	float vnormal;
	float impulse;
	float ix, iy;

	float2 vel;
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
				   __local float2 *local_result, __global float2 *group_result)
{
	float4 ball;
	float2 vel, local_vel;
	uint global_addr, local_addr;
	int a, group_id;

	global_addr = get_global_id(0);
	ball = data[global_addr];
	group_id = get_group_id(0);

	vel.s0 = 0;
	vel.s1 = 0;

	/*local_addr = get_local_id(0);
	local_result[local_addr] = local_vel;*/
	

	for (a = 0; a < POPSIZE; a++)
		if (a != group_id)
			if (ballCollision(ball, data[a]) == COLLIDE)
				vel = resolveCollision(group_id, ball, a, data[a]);

	group_result[get_group_id(0)] = vel;
}