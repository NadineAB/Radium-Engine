#include <FancyMeshComponent.hpp>

#include <iostream>

#include <Core/String/StringUtils.hpp>
#include <Core/Mesh/MeshUtils.hpp>

#include <Core/Geometry/Normal/Normal.hpp>

#include <Engine/Renderer/RenderObject/RenderObjectManager.hpp>
#include <Engine/Entity/ComponentMessenger.hpp>

#include <Engine/Renderer/Mesh/Mesh.hpp>
#include <Engine/Renderer/RenderObject/RenderObject.hpp>
#include <Engine/Renderer/RenderObject/RenderObjectTypes.hpp>
#include <Engine/Renderer/RenderTechnique/RenderTechnique.hpp>
#include <Engine/Renderer/RenderTechnique/ShaderConfiguration.hpp>
#include <Engine/Renderer/RenderTechnique/ShaderProgramManager.hpp>
#include <Engine/Renderer/RenderObject/Primitives/DrawPrimitives.hpp>

#include <Engine/Assets/FileData.hpp>
#include <Engine/Assets/GeometryData.hpp>

namespace FancyMeshPlugin
{
    FancyMeshComponent::FancyMeshComponent(const std::string& name , bool deformable)
        : Ra::Engine::Component( name  ) , m_deformable(deformable)
    {
    }

    FancyMeshComponent::~FancyMeshComponent()
    {
        // TODO(Charly): Should we ask the RO manager to delete our render object ?
    }

    void FancyMeshComponent::initialize()
    {
    }

    void FancyMeshComponent::addMeshRenderObject( const Ra::Core::TriangleMesh& mesh, const std::string& name )
    {
        Ra::Engine::RenderTechnique* technique = new Ra::Engine::RenderTechnique;
        technique->material = new Ra::Engine::Material( "Default" );
        technique->shaderConfig = Ra::Engine::ShaderConfiguration( "Default", "../Shaders" );

        addMeshRenderObject(mesh, name, technique);
    }

    void FancyMeshComponent::addMeshRenderObject( const Ra::Core::TriangleMesh& mesh,
                                                  const std::string& name,
                                                  Ra::Engine::RenderTechnique* technique )
    {
        m_mesh = mesh;
        setupIO(name);

        Ra::Engine::RenderObject* renderObject = new Ra::Engine::RenderObject( name, this, Ra::Engine::RenderObjectType::FANCY );
        renderObject->setVisible( true );
        renderObject->setRenderTechnique( technique );

        std::shared_ptr<Ra::Engine::Mesh> displayMesh( new Ra::Engine::Mesh( name ) );
        std::vector<uint> indices;
        indices.reserve( mesh.m_triangles.size() * 3 );
        for ( const auto& i : mesh.m_triangles )
        {
            indices.push_back( i.x() );
            indices.push_back( i.y() );
            indices.push_back( i.z() );
        }

        displayMesh->loadGeometry( mesh.m_vertices, indices );
        displayMesh->addData( Ra::Engine::Mesh::VERTEX_NORMAL, mesh.m_normals );

        renderObject->setMesh( displayMesh );

        addRenderObject( renderObject );
    }

    void FancyMeshComponent::handleMeshLoading( const Ra::Asset::GeometryData* data )
    {
        std::string name( m_name );
        name.append( "_" + data->getName() );

        std::string roName = name;
        roName.append( "_RO" );

        std::string meshName = name;
        meshName.append( "_Mesh" );

        std::string matName = name;
        matName.append( "_Mat" );

        m_contentName = data->getName();

        Ra::Engine::RenderObject* renderObject = new Ra::Engine::RenderObject( roName, this, Ra::Engine::RenderObjectType::FANCY );
        renderObject->setVisible( true );

        std::shared_ptr<Ra::Engine::Mesh> mesh( new Ra::Engine::Mesh( meshName ) );




        m_mesh.clear();
        for( const auto& v : data->getVertices() )
        {
            m_mesh.m_vertices.push_back( data->getFrame() * v );
        }
        for( const auto& face : data->getFaces() ) {
            m_mesh.m_triangles.push_back( Ra::Core::Vector3i( face[0], face[1], face[2] ) );
        }
        Ra::Core::Geometry::uniformNormal( m_mesh.m_vertices, m_mesh.m_triangles, m_mesh.m_normals );

        setupIO( data->getName());


        // FIXME(Charly): Find a cleaner way to build geometry
        std::vector<uint> indices;
        for ( const auto& face : data->getFaces() )
        {
            indices.push_back( face[0] );
            indices.push_back( face[1] );
            indices.push_back( face[2] );
        }

        Ra::Core::Vector3Array positions;
        Ra::Core::Vector3Array normals;
        Ra::Core::Vector4Array tangents;
        Ra::Core::Vector4Array bitangents;
        Ra::Core::Vector4Array texcoords;
        Ra::Core::Vector4Array colors;

        positions = m_mesh.m_vertices;
        normals   = m_mesh.m_normals;
        for ( const auto& v : data->getTangents() )     tangents.push_back( v );
        for ( const auto& v : data->getBiTangents() )   bitangents.push_back( v );
        for ( const auto& v : data->getTexCoords() )    texcoords.push_back( v );
        for ( const auto& v : data->getColors() )       colors.push_back( v );

        mesh->loadGeometry( positions, indices );


        mesh->addData( Ra::Engine::Mesh::VERTEX_NORMAL, normals );
        mesh->addData( Ra::Engine::Mesh::VERTEX_TANGENT, tangents );
        mesh->addData( Ra::Engine::Mesh::VERTEX_BITANGENT, bitangents );
        mesh->addData( Ra::Engine::Mesh::VERTEX_TEXCOORD, texcoords );
        mesh->addData( Ra::Engine::Mesh::VERTEX_COLOR, colors );
        // FIXME(Charly): Should not weights be part of the geometry ?
        //        mesh->addData( Ra::Engine::Mesh::VERTEX_WEIGHTS, meshData.weights );

        renderObject->setMesh( mesh );

        m_meshIndex = addRenderObject(renderObject);

        // FIXME(Charly)
        // Build m_mesh
        //        int triangleCount = meshData.indices.size() / 3;
        //        int vertexCount = meshData.positions.size();
        //        m_mesh.m_vertices.resize(vertexCount);
        //        m_mesh.m_normals.resize(vertexCount);
        //        m_mesh.m_triangles.resize(triangleCount);

        //        for (int i = 0; i < vertexCount; i++)
        //        {
        //            Ra::Core::Vector4 pos = meshData.positions[i];
        //            Ra::Core::Vector4 normals = meshData.normals[i];
        //            m_mesh.m_vertices[i] = Ra::Core::Vector3(pos(0), pos(1), pos(2));
        //            m_mesh.m_normals[i] = Ra::Core::Vector3(normals(0), normals(1), normals(2));
        //        }

        //        for (int i = 0; i < triangleCount; i++)
        //            m_mesh.m_triangles[i] = Ra::Core::Triangle(meshData.indices[i * 3], meshData.indices[i * 3 + 1], meshData.indices[i * 3 + 2]);

        Ra::Engine::RenderTechnique* rt = new Ra::Engine::RenderTechnique;
        Ra::Engine::Material* mat = new Ra::Engine::Material( matName );
        auto m = data->getMaterial();
        if ( m.hasDiffuse() )   mat->setKd( m.m_diffuse );
        if ( m.hasSpecular() )  mat->setKs( m.m_specular );
        if ( m.hasShininess() ) mat->setNs( m.m_shininess );
        //if ( m.hasDiffuseTexture() ) mat->addTexture( Ra::Engine::Material::TextureType::TEX_DIFFUSE, m.m_texDiffuse );
        //if ( m.hasSpecularTexture() ) mat->addTexture( Ra::Engine::Material::TextureType::TEX_SPECULAR, m.m_texSpecular );
        //if ( m.hasShininessTexture() ) mat->addTexture( Ra::Engine::Material::TextureType::TEX_SHININESS, m.m_texShininess );
        //if ( m.hasOpacityTexture() ) mat->addTexture( Ra::Engine::Material::TextureType::TEX_ALPHA, m.m_texOpacity );
        //if ( m.hasNormalTexture() ) mat->addTexture( Ra::Engine::Material::TextureType::TEX_NORMAL, m.m_texNormal );

        rt->material = mat;
        rt->shaderConfig = Ra::Engine::ShaderConfiguration( "BlinnPhong", "../Shaders" );

        renderObject->setRenderTechnique( rt );

    }

    Ra::Core::Index FancyMeshComponent::getRenderObjectIndex() const
    {
        return m_meshIndex;
    }

    Ra::Core::TriangleMesh FancyMeshComponent::getMesh() const
    {
        return m_mesh;
    }

    void FancyMeshComponent::setupIO(const std::string& id)
    {
        Ra::Engine::ComponentMessenger::GetterCallback cbOut = std::bind( &FancyMeshComponent::getMeshOutput, this );
        Ra::Engine::ComponentMessenger::getInstance()->registerOutput<Ra::Core::TriangleMesh>( getEntity(), this, id, cbOut);

        if( m_deformable)
        {
            Ra::Engine::ComponentMessenger::SetterCallback cbIn = std::bind( &FancyMeshComponent::setMeshInput, this, std::placeholders::_1 );
            Ra::Engine::ComponentMessenger::getInstance()->registerInput<Ra::Core::TriangleMesh>( getEntity(), this, id, cbIn);
        }

    }

    const void* FancyMeshComponent::getMeshOutput() const
    {
        return &m_mesh;
    }

    void FancyMeshComponent::setMeshInput(const void *meshptr)
    {

        CORE_ASSERT( meshptr, " Input is null");
        CORE_ASSERT( m_deformable, "Mesh is not deformable");

        m_mesh = *(static_cast<const Ra::Core::TriangleMesh*>(meshptr));

        // TODO : factor with code loading a mesh.
        std::vector<uint> indices;
        indices.reserve( m_mesh.m_triangles.size() * 3 );
        for ( const auto& i : m_mesh.m_triangles )
        {
            indices.push_back( i.x() );
            indices.push_back( i.y() );
            indices.push_back( i.z() );
        }

        const auto& ro =getRoMgr()->update(getRenderObjectIndex(), false);
        auto displayMesh = ro->getMesh();
        displayMesh->loadGeometry( m_mesh );
        displayMesh->addData( Ra::Engine::Mesh::VERTEX_NORMAL, m_mesh.m_normals );
        getRoMgr()->doneUpdating(getRenderObjectIndex());

    }

    void FancyMeshComponent::rayCastQuery( const Ra::Core::Ray& r) const
    {
        auto result  = Ra::Core::MeshUtils::castRay( m_mesh, r );
        int tidx = result.m_hitTriangle;
        if (tidx >= 0)
        {
            LOG(logINFO) << " Hit triangle " << tidx;
            LOG(logINFO) << " Nearest vertex " << result.m_nearestVertex;
        }
    }

} // namespace FancyMeshPlugin