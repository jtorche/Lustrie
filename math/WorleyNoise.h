#pragma once

#include "core/type.h"
#include "core/ImageAlgorithm.h"
#include <random>
#include <algorithm>
#include <memory>
#include <deque>

namespace tim
{
    template<class T>
    class WorleyNoise
    {
    public:
        using Point = T;
        WorleyNoise(uint32_t nbPoints, int nth = 1, int seed = 42);

        WorleyNoise(const WorleyNoise&) = delete;
        WorleyNoise& operator=(const WorleyNoise&) = delete;

        WorleyNoise(WorleyNoise&&) = default;
        WorleyNoise& operator=(WorleyNoise&&) = default;

        float noise(T x) const;

    private:
        int _nth;
        std::vector<T> _points;
        float _sqrtNbPoints;

        struct Node
        {
            Node *left, *right;
            std::vector<uint32_t> container;
        };

        std::deque<Node> _nodePool;
        Node* _root = nullptr;

        void optimiseSpace(Node*, Point, float, uint32_t depth, uint32_t maxDepth);
        float search(Point, Node*, Point, float, uint32_t depth) const;
    };

    template<class T>
    struct WorleyNoiseInstancer
    {
        uint32_t _nbPoints, _layerCoef;
        int _nth, _seed;

        WorleyNoiseInstancer(uint32_t nbPoints, uint32_t layerCoef = 3, uint32_t nth = 1, int seed = 42) : _nbPoints(nbPoints), _layerCoef(layerCoef), _nth(nth), _seed(seed) {}

        T operator()(uint32_t layer) const
        {
            return T(_nbPoints * uipow(_layerCoef, layer), _nth, _seed+layer);
        }
    };

    /********************/
    /*** Implentation ***/
    /********************/

    /** Worley Noise **/

    template<class T> WorleyNoise<T>::WorleyNoise(uint32_t nbPoints, int nth, int seed) : _nth(nth), _nodePool{}, _root{&_nodePool.emplace_back()}
    {
        std::mt19937 randEngine(seed);
        std::uniform_real_distribution<float> random(0,1);

        for(uint32_t i=0 ; i<nbPoints ; ++i)
        {
            T v;
            for(uint32_t j=0 ; j<T::Length ; ++j)
                v[j] = random(randEngine);
            _points.push_back(v);
        }

        int maxDepth = int(log2f(float(nbPoints)) / Point::Length + 0.5f);
        maxDepth = std::min(8, std::max(0, maxDepth));
        //std::cout << "maxdepth:" << maxDepth << " nbPts:" << nbPoints<<std::endl;
        optimiseSpace(_root, Point(0.5f), 0.5f, 0, maxDepth);

        _sqrtNbPoints = powf(float(nbPoints), 1.f/float(Point::Length));
    }

    template<class T> float WorleyNoise<T>::noise(T x) const
    {
        x.apply([](float val) { return fmodf(fabsf(val), 1.f); });

        float result=0;
        if(_root == nullptr)
        {
            int nth = _nth>0 ? _nth:1;
            std::vector<float> dists(_points.size());
            for(uint32_t i=0 ; i<_points.size() ; ++i)
                dists[i] = (_points[i] - x).length2();

            std::partial_sort(dists.begin(), dists.begin()+nth, dists.end());

            result = sqrtf(dists[nth-1]);
        }
        else
            result = search(x, _root, Point(0.5f), 0.5f, 0);

        return std::min(result*_sqrtNbPoints, 1.f);
    }

    namespace
    {
        template<class T> bool isIn(T p, T center, float size)
        {
            p -= center;
            for(uint32_t i=0 ; i<T::Length ; ++i)
            {
                if(fabsf(p[i]) > size)
                    return false;
            }
            return true;
        }
    }

    template<class T> void WorleyNoise<T>::optimiseSpace(Node* node, Point center, float size, uint32_t depth, uint32_t maxDepth)
    {
        if(depth % Point::Length==0)
        {
            for(uint32_t i=0 ; i<_points.size() ; ++i)
            {
                if(isIn(_points[i], center, size*2))
                    node->container.push_back(i);
            }
        }

        if(depth/Point::Length == maxDepth || (node->container.empty() && depth % Point::Length==0))
        {
            node->left = nullptr;
            node->right = nullptr;
        }
        else
        {
            int state = depth % Point::Length;
            node->left = &_nodePool.emplace_back();
            node->right = &_nodePool.emplace_back();

            Point lcenter = center, rcenter = center;
            lcenter[state] -= size*0.5f;
            rcenter[state] += size*0.5f;

            if(depth % Point::Length == Point::Length-1)
                size *= 0.5f;

            optimiseSpace(node->left, lcenter, size, depth+1, maxDepth);
            optimiseSpace(node->right, rcenter, size, depth+1, maxDepth);
        }
    }

    template<class T> float WorleyNoise<T>::search(Point p, Node* node, Point center, float size, uint32_t depth) const
    {
        int state = depth % Point::Length;
        Node* next_node = nullptr;
        if(p[state] < center[state])
        {
            center[state] -= size*0.5f;
            next_node = node->left;
        }
        else
        {
            center[state] += size*0.5f;
            next_node = node->right;
        }

        if(depth % Point::Length == Point::Length-1)
            size *= 0.5f;

        if(depth%Point::Length == 0)
        {
            float d = -1;
            if(next_node)
                d = search(p, next_node, center, size, depth+1);

            if(d < 0)
            {
                int nth = _nth>0 ? _nth:1;
                std::vector<float> dists;

                for(uint32_t i=0 ; i<node->container.size() ; ++i)
                {
                    //uint32_t gg = node->container[i];
                    float dist = (_points[ node->container[i] ] - p).length2();

                    if(dist < size*size)
                        dists.push_back(dist);
                }

                if(int(dists.size()) < nth)
                    return -1;

                std::partial_sort(dists.begin(), dists.begin()+nth, dists.end());

                return sqrtf(dists[nth-1]);
            }
            else return d;
        }
        else
            return search(p, next_node, center, size, depth+1);
    }

}
