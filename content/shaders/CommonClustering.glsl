#ifndef _COMMON_CLUSTERING_GLSL_
#define _COMMON_CLUSTERING_GLSL_

uint CalculateClusterIndex(float linearDepth, float near, float far, uint maxClusters)
{
	return clamp(uint(float(maxClusters) * log(linearDepth/near) / log(far/near)), 0, maxClusters - 1); // clump it for zero based index just in case
}

#endif
