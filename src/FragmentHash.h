#ifndef fragment_hash_h
#define fragment_hash_h

#include <string>
#include <vector>
#include <memory>

#include "ShapeAndPositionInvariantImage.hpp"

using namespace std;

template <typename T> class FragmentHash
{
private:
protected:
    T hash_;
    ring_t shape_;
public:

    FragmentHash()
    {}

    FragmentHash(ShapeAndPositionInvariantImage image)
    {}

    FragmentHash(const string &conver, ring_t shape):
        shape_(shape)
    {
        //convert string to hash
    }

    FragmentHash(const FragmentHash& that):
        hash_(that.hash_),
        shape_(that.shape_)
    {}

    virtual string toString() = 0;

    //getters and setters

    virtual inline T getHash() const { return hash_; } 

    virtual ring_t getShape() const { return shape_; }

    virtual int getHammingDistance(const FragmentHash<T>& inHash) = 0;
};

#endif // fragment_hash_h
