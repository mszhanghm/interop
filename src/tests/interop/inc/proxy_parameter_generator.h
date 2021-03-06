/** Lazy parameter generation of a persistent std::vector
 *
 *  @note This code may only work with gtest 1.7.0
 *
 *  @file
 *  @date 6/23/16
 *  @version 1.0
 *  @copyright GNU Public License.
 */
#pragma once
#include <gtest/gtest.h>
#include "interop/util/type_traits.h"
#include "interop/util/static_assert.h"


namespace illumina{ namespace interop { namespace unittest
{

    /** Data structure that holds the parameters
     *
     * This holds a reference to the input vector, so ensure it is not on the stack!
     *
     * @note This code may only work with gtest 1.7.0
     */
    template<typename T, typename Proxy>
    class proxy_argument_generator : public ::testing::internal::ParamGeneratorInterface< typename T::parent_type >
    {
    public:
        /** Constructor
         *
         * @param obj object that acts like a functor
         * @param vec reference to persistent vector of values
         */
        proxy_argument_generator(T& obj, const std::vector<Proxy>& vec) : m_vec(vec), m_object(obj){}
        /** Destructor */
        virtual ~proxy_argument_generator(){}
        /** Iterator to start of parameter collection
         *
         * @return iterator to start of parameter collection
         */
        ::testing::internal::ParamIteratorInterface< typename T::parent_type >* Begin() const;
        /** Iterator to end of parameter collection
         *
         * @return iterator to end of parameter collection
         */
        ::testing::internal::ParamIteratorInterface< typename T::parent_type >* End() const;

    private:
        const std::vector<Proxy>& m_vec;
        T& m_object;
    };
    /** Iterator over persistent vector of arguments
     *
     * @note This code may only work with gtest 1.7.0
     */
    template<typename T, typename Proxy>
    class proxy_argument_iterator : public ::testing::internal::ParamIteratorInterface< typename T::parent_type >
    {
        typedef typename std::vector< Proxy >::const_iterator const_iterator;
    public:
        /** Constructor
         *
         * @param obj object-like functor
         * @param base generator base
         * @param it iterator to std::vector
         * @param it_end iterator to end of std::vector
         */
        proxy_argument_iterator(T& obj,
                                const proxy_argument_generator<T,Proxy>& base,
                                const_iterator it,
                                const_iterator it_end) :
                m_base(base), m_begin(it), m_current(it), m_end(it_end), m_object(obj)
        {
            static_assert(!is_pointer<T>::value, "This class does not free memory and should not take a pointer");
            if(m_current < m_end)
            {
                m_current_val=m_object(*m_current);
            }
        }
        /** Destructor */
        virtual ~proxy_argument_iterator()
        {
        }
        /** A pointer to the base generator instance.
         * Used only for the purposes of iterator comparison
         * to make sure that two iterators belong to the same generator.
         *
         * @return base generator
         */
        virtual const ::testing::internal::ParamGeneratorInterface< typename T::parent_type >* BaseGenerator() const
        {
            return &m_base;
        }
        /** Advances iterator to point to the next element
         * provided by the generator. The caller is responsible
         * for not calling Advance() on an iterator equal to
         * BaseGenerator()->End().
         *
         *
         */
        virtual void Advance()
        {
            if(m_current_val == 0 || m_current_val->advance())
            {
                ++m_current;
                if(m_current < m_end)
                {
                    m_current_val=m_object(*m_current);
                }
            }
        }
        /** Clones the iterator object. Used for implementing copy semantics
         * of ParamIterator<T>.
         *
         * @return new instance to object
         */
        virtual ::testing::internal::ParamIteratorInterface< typename T::parent_type >* Clone() const
        {
            return new proxy_argument_iterator(*this);
        }
        /** Dereferences the current iterator and provides (read-only) access
         * to the pointed value. It is the caller's responsibility not to call
         * Current() on an iterator equal to BaseGenerator()->End().
         * Used for implementing ParamGenerator<T>::operator*().
         *
         * @return pointer to current value
         */
        virtual const typename T::parent_type* Current() const
        {
            return &m_current_val;
        }
        /** Determines whether the given iterator and other point to the same
         * element in the sequence generated by the generator.
         * Used for implementing ParamGenerator<T>::operator==().
         *
         * @param other other iterator
         * @return true if the base and iterator match
         */
        virtual bool Equals(const ::testing::internal::ParamIteratorInterface< typename T::parent_type >& other) const
        {
            return &m_base == other.BaseGenerator() &&
                    m_current == dynamic_cast<const proxy_argument_iterator<T,Proxy>*>(&other)->m_current;
        }

    private:
        proxy_argument_iterator(const proxy_argument_iterator<T,Proxy>& other)
                : m_base(other.m_base), m_begin(other.m_begin),
                m_current(other.m_current), m_end(other.m_end), m_object(other.m_object)
        {
            if(m_current < m_end) m_current_val=m_object(*m_current);
        }

    private:
        const proxy_argument_generator<T,Proxy>& m_base;
        const const_iterator m_begin;
        const_iterator m_current;
        const_iterator m_end;
        T& m_object;
        mutable typename T::parent_type m_current_val;
    };


    template<typename T, typename Proxy>
    ::testing::internal::ParamIteratorInterface< typename T::parent_type >* proxy_argument_generator<T,Proxy>::Begin() const
    {
        return new proxy_argument_iterator<T,Proxy>(m_object, *this, m_vec.begin(), m_vec.end());
    }
    template<typename T, typename Proxy>
    ::testing::internal::ParamIteratorInterface< typename T::parent_type >* proxy_argument_generator<T,Proxy>::End() const
    {
        return new proxy_argument_iterator<T,Proxy>(m_object, *this, m_vec.end(), m_vec.end());
    }

    /** Generate parameters from a persistent vector (cannot be on the stack!)
     *
     * @note This code may only work with gtest 1.7.0
     * @param object object that acts like a functor
     * @param values list of values
     * @return parameter generator
     */
    template<typename T, typename Proxy>
    ::testing::internal::ParamGenerator< typename T::parent_type > ProxyValuesIn(T& object, const std::vector<Proxy>& values)
    {
        return ::testing::internal::ParamGenerator< typename T::parent_type >(new proxy_argument_generator<T, Proxy>(object, values));
    }
}}}

