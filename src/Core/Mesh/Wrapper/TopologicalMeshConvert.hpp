#ifndef MESHCONVERTER_H
#define MESHCONVERTER_H

#include <Core/Mesh/TopologicalTriMesh/topologicalmesh.h>
#include <Core/Mesh/TriangleMesh.hpp>

namespace Ra {
namespace Core {

    //! Adapter class to convert between vortex::Mesh and vortex::TopologicalMesh
    //! \todo take into account texture coordinates and normals more robustly.
    class RA_CORE_API MeshConverter{
    public:
        static void convert(TopologicalMesh& in, TriangleMesh& out);
        static void convert(const TriangleMesh& in, TopologicalMesh& out);
    };
}
}

#endif // MESHCONVERTER_H