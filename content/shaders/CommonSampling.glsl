#ifndef _COMMON_SAMPLING_GLSL_
#define _COMMON_SAMPLING_GLSL_

// thanks nVidia for your goodies

// Generate a random unsigned int from two unsigned int values, using 16 pairs
// of rounds of the Tiny Encryption Algorithm. See Zafar, Olano, and Curtis,
// "GPU Random Numbers via the Tiny Encryption Algorithm"
uint tea(uint val0, uint val1)
{
  uint v0 = val0;
  uint v1 = val1;
  uint s0 = 0;

  for(uint n = 0; n < 16; n++)
  {
    s0 += 0x9e3779b9;
    v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
    v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
  }

  return v0;
}

// Generate a random unsigned int in [0, 2^24) given the previous RNG state
// using the Numerical Recipes linear congruential generator
uint lcg(inout uint prev)
{
  uint LCG_A = 1664525u;
  uint LCG_C = 1013904223u;
  prev       = (LCG_A * prev + LCG_C);
  return prev & 0x00FFFFFF;
}

// Generate a random float in [0, 1) given the previous RNG state
float rnd(inout uint prev)
{
  return (float(lcg(prev)) / float(0x01000000));
}

//-------------------------------------------------------------------------------------------------

uint JenkinsHash(uint x)
{
    x += x << 10;
    x ^= x >> 6;
    x += x << 3;
    x ^= x >> 11;
    x += x << 15;
    return x;
}

uint InitRNG(uvec2 pixel , uvec2 resolution , uint frame)
{
    uint rngState = uint(dot(pixel , uvec2(1, resolution.x))) ^ JenkinsHash(frame);
    return JenkinsHash(rngState);
}

float UintToFloat(uint x)
{
    return uintBitsToFloat(0x3f800000 | (x >> 9)) - 1.f;
}

uint Xorshift(inout uint rngState)
{
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return rngState;
}

float Rand(inout uint rngState)
{
    return UintToFloat(Xorshift(rngState));
}


//-------------------------------------------------------------------------------------------------
// Sampling
//-------------------------------------------------------------------------------------------------

#define M_PI 3.141592

vec3 SphericalFibonacci(float i, float n) {
    const float PHI = sqrt(5) * 0.5f + 0.5f;
    float fraction = (i * (PHI - 1)) - floor(i * (PHI - 1));
    float phi = 2.f * M_PI * fraction;
    float cosTheta = 1.f - (2.f * i + 1.f) * (1.f / n);
    float sinTheta = sqrt(clamp(1.f - cosTheta*cosTheta, 0.0f, 1.0f));

    return vec3(cos(phi) * sinTheta , sin(phi) * sinTheta , cosTheta);
}

//Ray generateSphericalFibonacciRay(int index) {
//    float numRays = imageSize.x * imageSize.y;
//    return Ray(vec3(0.f), 0.f, sphericalFibonacci(index , numRays), inf);
//}

// Randomly sampling around +Z
vec3 SamplingHemisphere(inout uint seed, in vec3 x, in vec3 y, in vec3 z)
{
  float r1 = rnd(seed);
  float r2 = rnd(seed);
  float sq = sqrt(1.0 - r2);

  vec3 direction = vec3(cos(2 * M_PI * r1) * sq, sin(2 * M_PI * r1) * sq, sqrt(r2));
  direction      = direction.x * x + direction.y * y + direction.z * z;

  return direction;
}

// Return the tangent and binormal from the incoming normal
void CreateCoordinateSystem(in vec3 N, out vec3 Nt, out vec3 Nb)
{
  if(abs(N.x) > abs(N.y))
    Nt = vec3(N.z, 0, -N.x) / sqrt(N.x * N.x + N.z * N.z);
  else
    Nt = vec3(0, -N.z, N.y) / sqrt(N.y * N.y + N.z * N.z);
  Nb = cross(N, Nt);
}

#endif
