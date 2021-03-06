#ifndef RADIUMENGINE_CORE_ALIGNED_STD_VECTOR_HPP_
#define RADIUMENGINE_CORE_ALIGNED_STD_VECTOR_HPP_
#include <Core/RaCore.hpp>

#include <vector>
#include <Eigen/StdVector> 

namespace Ra
{
    namespace Core
    {

        /// Shortcut for the ubiquitous aligned std::vector
        /// Uses Eigen's aligned allocator, as stated in 
        /// http://eigen.tuxfamily.org/dox/group__TopicStlContainers.html
        template <typename T>
        class AlignedStdVector : public std::vector <T, Eigen::aligned_allocator<T>> 
        {
            using std::vector<T, Eigen::aligned_allocator<T>>::vector;
        };
    }
}
#endif //RADIUMENGINE_CORE_ALIGNED_STD_VECTOR_HPP_
