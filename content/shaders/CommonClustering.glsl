#ifndef _COMMON_CLUSTERING_GLSL_
#define _COMMON_CLUSTERING_GLSL_

uint CalculateClusterDepthIndex(float linearDepth, float near, float far, uint maxClusters)
{
	return clamp(uint(float(maxClusters) * log(linearDepth/near) / log(far/near)), 0, maxClusters - 1); // clump it for zero based index just in case
}

uvec2 ClusterCoordsFromUV(vec2 uv, vec2 clusterScreenOverflow, uvec2 numClusters)
{
	vec2 clusteredUV = (uv + 0.5f * (clusterScreenOverflow - vec2(1.0f))) / clusterScreenOverflow;
    uint clusterX = uint(clamp(clusteredUV.x * numClusters.x, 0, numClusters.x - 1));
	uint clusterY = uint(clamp(clusteredUV.y * numClusters.y, 0, numClusters.y - 1));
	return uvec2(clusterX, clusterY);
}

#endif
