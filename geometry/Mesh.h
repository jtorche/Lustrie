#pragma once

#include <vector>
#include <array>
#include <string>

#include <fstream>

#include "math/Vector.h"
#include "math/Quaternion.h"
#include "core/ImageAlgorithm.h"
#include "math/Sphere.h"

namespace tim
{
    class Curve;
    class BaseMesh
	{
        friend class Curve;

    public:
        struct Face
        {
            std::array<uint32_t,4> indexes; // support quads triangles, lines and points
            int nbIndexes;
        };

	public:
        BaseMesh() = default;
        ~BaseMesh() = default;
		
        BaseMesh(const BaseMesh&) = default;
        BaseMesh(BaseMesh&&) = default;
		
        BaseMesh& operator=(const BaseMesh&) = default;
        BaseMesh& operator=(BaseMesh&&) = default;

        BaseMesh scaled(vec3) const; // scale by a vec3 each vertice
        BaseMesh translated(vec3) const; // translate by a vec3 each vertice
        BaseMesh transformed(const mat4&) const;
        BaseMesh rotated(Quat) const;
        BaseMesh rotated(const mat3&) const;

        template<class T> BaseMesh& mapVertices(const T&);
        template<class T> BaseMesh& mapNormals(const T&);

        uint32_t nbVertices() const;
		uint32_t nbFaces() const;
        vec3 position(uint32_t) const;

		const vec3* vertexData() const;
		std::vector<uint32_t> indexData(uint32_t nbPointsInFace = 3) const;

		uint32_t requestBufferSize(bool withNormal = true, bool withUv = false) const;
		void fillBuffer(void*, bool withNormal = true, bool withUv = false) const;

        void clearFaces();
        BaseMesh& addFace(const Face&);

        BaseMesh& operator+=(const BaseMesh&);

        void exportToObj(std::string) const;

        BaseMesh& computeNormals(bool correctSeems = true, int smooth = 0);
        BaseMesh& invertNormals();
        BaseMesh& invertFaces();

		static void computeJoinNormals(std::vector<BaseMesh*>&, int smooth = 0);

		vec3 vertex(uint32_t) const;
		vec3 normal(uint32_t) const;

		Sphere computeBoundingSphere();
		
    protected:
		std::vector<vec3> _vertices;
        std::vector<Face> _faces;
        std::vector<vec3> _normals;
        std::vector<vec2> _texCoords;

        std::vector<std::vector<uint32_t>> _vertexToFaces; // for each vertex, we know the faces the vertex is in

        void buildVertexFaceMap(bool useRealPosition);

   private:
        std::ofstream& writeVertex(std::ofstream&, uint32_t) const;

        vec3 faceNormal(uint32_t) const;

	protected:
		static void generateGrid(BaseMesh&, vec2 size, uivec2 resolution, const ImageAlgorithm<float>&, float Zscale, bool withUV, bool triangulate);
	};

    inline BaseMesh& BaseMesh::addFace(const Face& face) { _faces.push_back(face); return *this; }
    inline uint32_t BaseMesh::nbVertices() const { return (uint32_t)_vertices.size(); }
	inline uint32_t BaseMesh::nbFaces() const { return (uint32_t)_faces.size(); }
	inline const vec3* BaseMesh::vertexData() const { return _vertices.data(); }

	inline vec3 BaseMesh::vertex(uint32_t index) const { return _vertices[index]; }
	inline vec3 BaseMesh::normal(uint32_t index) const { return _normals[index]; }

    template<class T> BaseMesh& BaseMesh::mapVertices(const T& tr)
    {
        for(auto& v : _vertices)
            v = tr(v);
        return *this;
    }

    template<class T> BaseMesh& BaseMesh::mapNormals(const T& tr)
    {
        for(auto& n : _normals)
            n = tr(n);
        return *this;
    }

	/* Mesh */

    class Mesh : public BaseMesh
    {
    public:
		using Vertex = vec3;

        Mesh() = default;
        ~Mesh() = default;
        Mesh(const Mesh&) = default;
        Mesh(Mesh&&) = default;

        Mesh(const BaseMesh& mesh);

        Mesh& operator=(const Mesh&) = default;
        Mesh& operator=(Mesh&&) = default;

        Mesh& addVertex(vec3);
		Mesh& addVertexAndNormal(vec3, vec3);

        Mesh& constructLine(vec3, vec3);
        Mesh& constructTriangle(vec3, vec3, vec3);
        Mesh& constructQuad(vec3, vec3, vec3, vec3);

		static Mesh generateGrid(vec2 size, uivec2 resolution, const ImageAlgorithm<float>&, float Zscale, bool triangulate = false);
    };

	inline Mesh& Mesh::addVertex(vec3 v) { _vertices.push_back(v); return *this; }
	inline Mesh& Mesh::addVertexAndNormal(vec3 v, vec3 n) { _vertices.push_back(v); _normals.push_back(n); return *this; }


	/* UV Mesh */

    class UVMesh : public BaseMesh
    {
    public:
        struct Vertex{ vec3 v; vec2 uv; };

        UVMesh() = default;
        ~UVMesh() = default;
        UVMesh(const UVMesh&) = default;
        UVMesh(UVMesh&&) = default;

        UVMesh(const BaseMesh& mesh);

        UVMesh& operator=(const UVMesh&) = default;
        UVMesh& operator=(UVMesh&&) = default;

        UVMesh& addVertex(const Vertex&);

        UVMesh& constructLine(const Vertex&, const Vertex&);
        UVMesh& constructTriangle(const Vertex&, const Vertex&, const Vertex&);
        UVMesh& constructQuad(const Vertex&, const Vertex&, const Vertex&, const Vertex&);

        UVMesh uv_scaled(vec2) const;

		static UVMesh generateGrid(vec2 size, uivec2 resolution, const ImageAlgorithm<float>&, float Zscale, bool triangulate = false);
    };

	inline UVMesh& UVMesh::addVertex(const Vertex& v) { _vertices.push_back(v.v); _texCoords.push_back(v.uv); return *this; }
}
