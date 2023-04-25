
/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
#ifndef GEOMODELUTILITIES_TRANSIENTCONSTSHAREDPTR_H
#define GEOMODELUTILITIES_TRANSIENTCONSTSHAREDPTR_H

#include <memory>
namespace GeoModel {
    /// The TransientConstSharedPtr allows non-const access if the pointer itself
    /// is non-const but in the const case only a const pointer is returned
    /// The class takes the ownership of the object using a shared_ptr
    template <typename Obj> class TransientConstSharedPtr {
    public:
        /// Get (non-const) access to the underlying object
        Obj* get() { return m_ptr.get(); }
        Obj* operator->() { return get(); }
        /// Get const access to the underlying object
        const Obj* get() const { return m_ptr.get(); }
        const Obj* operator->() const { return get(); }

        /// Constructor from unique_ptr
        TransientConstSharedPtr(std::unique_ptr<Obj> ptr) : m_ptr{std::move(ptr)} {}
        /// Constructor from shared_ptr
        TransientConstSharedPtr(std::shared_ptr<Obj> ptr) : m_ptr{std::move(ptr)} {}
        /// Constructor from raw ptr
        TransientConstSharedPtr(Obj* ptr) : m_ptr{ptr} {}

        /// Standard constructor
        TransientConstSharedPtr() = default;
        /// Delete the copy constructor if the object is const
        TransientConstSharedPtr(const TransientConstSharedPtr& other) = default;
        /// Standard move constructor
        TransientConstSharedPtr(TransientConstSharedPtr&& other) = default;

        /// Assignment operator
        TransientConstSharedPtr& operator=(const TransientConstSharedPtr& other) = default;
        TransientConstSharedPtr& operator=(const std::shared_ptr<Obj>& other) {
            m_ptr = other.m_ptr;
            return *this;
        }
        /// Move assignment operator
        TransientConstSharedPtr& operator=(TransientConstSharedPtr&& other) = default;
        TransientConstSharedPtr& operator=(std::unique_ptr<Obj>&& other) {
            m_ptr = std::move(other);
            return *this;
        }
        TransientConstSharedPtr& operator=(std::shared_ptr<Obj>&& other) {
            m_ptr = std::move(other);
            return *this;
        }
        /// Overload the pointer
        void reset(std::unique_ptr<Obj> newObj = nullptr) { m_ptr = std::move(newObj); }
        /// Release the memory
        std::shared_ptr<const Obj> release() { return std::move(m_ptr); }
        /// Is the pointer defined
        operator bool() const { return m_ptr.get() != nullptr; }
        bool operator!() const { return !m_ptr; }

        /// Smaller operator to insert the pointer into sets
        bool operator<(const TransientConstSharedPtr& other) const { return m_ptr.get() < other.m_ptr.get(); }

    private:
        std::shared_ptr<Obj> m_ptr{};
    };
}  // namespace GeoModel
#endif