#pragma once

#include <vector>
#include <algorithm>
#include "math/Vector.h"
#include "math/Matrix.h"

#include "geometry/Mesh.h"
#include <iostream>

namespace tim
{
	class Curve
	{
	public:
        Curve() = default;
        ~Curve() = default;

        Curve(const Curve&) = default;
        Curve(Curve&&) = default;

        Curve& operator=(const Curve&) = default;
        Curve& operator=(Curve&&) = default;

        void setClosed(bool);
        bool isClosed() const;
        uint32_t numPoints() const;
		vec3 point(uint32_t) const;
        float radius(uint32_t) const;

		vec3 computeDirection(uint32_t) const;

        Curve& addPoint(vec3);
        Curve& addPoint(vec3, float);

        Mesh convertToWireMesh() const;

        template<class RadiusFun> // radiusFun(time, angle)
        Mesh convertToMesh(const RadiusFun&,  uint32_t resolution, bool mergeLast, bool triangle) const;

        template<class RadiusFun> // radiusFun(time, angle)
        UVMesh convertToUVMesh(const RadiusFun&,  uint32_t resolution, bool mergeLast, bool triangle, bool uniform_uv = false) const;

        Mesh convertToMesh(uint32_t resolution = 4, bool mergeLast = false, bool triangle = true) const;
        UVMesh convertToUVMesh(uint32_t resolution = 4, bool mergeLast = false, bool triangle = true, bool uniform_uv = false) const;

        template<class F1, class F2, class F3>
        static Curve parametrization(vec2 range, int numPoints, const F1&&, const F2&&, const F3&&);

    private:
        std::vector<vec4> _points;
        bool _closed = false;

    private:
        static void tesselateCylindre(BaseMesh&, const std::vector<uint32_t>& bottom, const std::vector<uint32_t>& top, uint32_t resolution, bool triangulate, bool cut);
        static void tesselateCone(BaseMesh&, const std::vector<uint32_t>& bottom, uint32_t top, uint32_t resolution, bool cut);

        static void aligneCircle(const std::vector<std::pair<vec3,vec2>>&, std::vector<std::pair<vec3,vec2>>&);

        template<class RadiusFun, class TypeMesh>
        TypeMesh convertToMesh(const RadiusFun&,  uint32_t resolution, bool mergeLast, bool triangle, bool cut, bool uniform_uv=false) const;
	};

    inline void Curve::setClosed(bool b) { _closed = b; }
    inline bool Curve::isClosed() const { return _closed; }

    inline uint32_t Curve::numPoints() const { return (uint32_t)_points.size(); }
    inline Curve& Curve::addPoint(vec3 p) { _points.push_back(vec4(p,0)); return *this; }
    inline Curve& Curve::addPoint(vec3 p, float rad) { _points.push_back(vec4(p,rad)); return *this; }

    inline vec3 Curve::point(uint32_t index) const { if (index >= _points.size()) return vec3(); return _points[index].to<3>(); }
    inline float Curve::radius(uint32_t index) const { if (index >= _points.size()) return 0; return _points[index].w(); }

    template<class F1, class F2, class F3>
    Curve Curve::parametrization(vec2 range, int numPoints, const F1&& f1, const F2&& f2, const F3&& f3)
    {
        Curve curve;
        float step = (range.y() - range.x()) / numPoints;
        for(int i=0 ; i<numPoints ; ++i)
        {
            curve.addPoint({f1(step*static_cast<float>(i)), f2(step*static_cast<float>(i)), f3(step*static_cast<float>(i))});
        }

        return curve;
    }

    template<class RadiusFun>
    Mesh Curve::convertToMesh(const RadiusFun& fun,  uint32_t resolution, bool mergeLast, bool triangle) const
    {
        return convertToMesh<RadiusFun, Mesh>(fun, resolution, mergeLast, triangle, false);
    }

    template<class RadiusFun>
    UVMesh Curve::convertToUVMesh(const RadiusFun& fun,  uint32_t resolution, bool mergeLast, bool triangle, bool uniform_uv) const
    {
        return convertToMesh<RadiusFun, UVMesh>(fun, resolution, mergeLast, triangle, true, uniform_uv);
    }

	namespace 
	{
		template<class T> struct AddVertex {};
        template<>       struct AddVertex<Mesh> { static void add(Mesh& m, vec3 pos, vec2) { m.addVertex(pos); } };
        template<>       struct AddVertex<UVMesh> { static void add(UVMesh& m, vec3 pos, vec2 uv) { m.addVertex({ pos,uv }); } };
	}

    template<class RadiusFun, class TypeMesh>
    TypeMesh Curve::convertToMesh(const RadiusFun& fun,  uint32_t resolution, bool mergeLast, bool triangle, bool cut, bool uniform_uv) const
    {
		if (_closed)
			mergeLast = false;

		TypeMesh mesh;

		if (resolution < 3 || _points.size() <= 2)
			return mesh;

        float timeStep = 1.f / (_points.size()-1);
        std::vector<std::vector<uint32_t>> pointIndexes(_points.size(), std::vector<uint32_t>(resolution+(cut ? 1:0)));
		uint32_t curIndex = 0;

        std::vector<std::pair<vec3,vec2>> prevPts;
        float accSizeCurve = 0;

		for (uint32_t i = 0; i<_points.size() + 1; ++i)
		{
			if (i == _points.size() && !_closed)
				break;

			vec3 dir;
			if (_closed)
                dir = (_points[(i + 1) % _points.size()].to<3>() - _points[pmod(static_cast<int>(i) - 1, (int)_points.size())].to<3>());
			else
                dir = (_points[std::min<uint32_t>(i + 1, (uint32_t)_points.size() - 1)].to<3>() - _points[i == 0 ? 0 : i - 1].to<3>());

            float sizeStep = dir.length();
            accSizeCurve += sizeStep;
            dir /= sizeStep;

			mat3 base = mat3::changeBasis(dir);

			if (!mergeLast || i < _points.size() - 1)
			{
				if (i < _points.size())
				{
                    std::vector<std::pair<vec3,vec2>> newPts;
                    float accAroundCurve=0;
                    for (uint32_t j = 0; j < resolution+(cut ? 1:0); ++j)
					{
                        if(j < resolution)
                        {
                            const float theta_norm = static_cast<float>(j) / resolution;
                            const float theta = theta_norm * 2 * PI;

                            vec3 p = _points[i].to<3>() + base * (vec3(cosf(theta), sinf(theta), 0.f) * fun(timeStep * i, theta_norm, i));
                            vec2 uv = uniform_uv ? vec2(theta_norm, timeStep * i) : vec2(TAU*_points[i].w() * theta_norm, accSizeCurve);

                            newPts.push_back({p,uv});
                            accAroundCurve += _points[i].w();
                        }

						pointIndexes[i][j] = curIndex + j;
					}

                    aligneCircle(prevPts, newPts);

                    for(auto p : newPts)
                        AddVertex<TypeMesh>::add(mesh, p.first, p.second);

                    if(cut)
                        AddVertex<TypeMesh>::add(mesh, newPts[0].first, {TAU*_points[i].w(), newPts[0].second.y()});

                    prevPts = newPts;
                }

				if (i > 0)
                    tesselateCylindre(mesh, pointIndexes[pmod(static_cast<int>(i) - 1, (int)_points.size())], pointIndexes[i%_points.size()], resolution, triangle, cut);
			}
			else // we need to create a unique point closing the mesh
			{
                vec3 p = _points[i%_points.size()].to<3>() + base * (vec3::construct(0) * fun(timeStep * i, 0, i));
                vec2 uv = vec2(0, accSizeCurve /** timeStep * i*/);
				AddVertex<TypeMesh>::add(mesh, p, uv);
                tesselateCone(mesh, pointIndexes[i - 1], curIndex, resolution, cut);
			}

            curIndex += (resolution + (cut ? 1:0));
		}

		return mesh;
    }
}
