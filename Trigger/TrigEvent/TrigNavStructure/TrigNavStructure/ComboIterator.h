/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGNAVSTRUCTURE_ComboIterator_H
#define TRIGNAVSTRUCTURE_ComboIterator_H

#include <vector>


namespace HLT {
  class TriggerElement;
  typedef std::vector<HLT::TriggerElement*> TEVec;

  class TrigNavStructure;


  /**
   * @brief Base class for iterator used to loop over multi-particle combinations.
   *
   * @author Carlo Schiavi  <Carlo.Schiavi@ge.infn.it>  -  INFN Genova
   *
   * The iterator classes inheriting this interface are used by HLT algorithms to iterate over
   * multi-particle combinations.
   */

  class ComboIteratorBase {

  public:

    /** @brief Constructor. */
    ComboIteratorBase() {}

    /** @brief Destructor. */
    virtual ~ComboIteratorBase() {}

    /** @brief Rewind method, resetting the iterator to the first element. */
    virtual bool rewind() = 0;

    /** @brief Unary * operator, used to recover the current combination. */
    virtual TEVec& operator*() = 0;

    /** @brief Validity check for the iterator.
     *  @return result of the validity check; returns false if iterator is at end, true otherwise.
     */
    virtual bool isValid() const = 0;

    /** @brief Accessor method for the current combination. */
    virtual TEVec& combination() = 0;

    /** @brief Post increment operator. */
    virtual ComboIteratorBase& operator++(int) = 0;

    /** @brief Pre increment operator. */
    virtual ComboIteratorBase& operator++() = 0;
   
  };


  /**
   * @brief Iterator used to loop over multi-particle combinations.
   *
   * @author Nicolas Berger  <Nicolas.Berger@cern.ch>  - CERN
   *
   * This iterator class is used by HLT algorithms to iterate over multi-particle combinations.
   * Given a set of TEs, passed in the constructor, the iterator can be used to move over all the
   * possible combinations of TEs. These combinations are built taking one TE from each of the
   * vector of TEs passed in the constructor. So, as an example, to build all possible couples
   * of TEs of the same type, the starting point will be a vector containing twice the same
   * vector of TEs.
   */

  class ComboIterator : public ComboIteratorBase {

  public:
    /**
     * @brief Constructor; gets a vector of TE vectors and a pointer to Navigation as arguments.
     * @param tes vector of subvectors of TEs, one for each input type. Each subvector is the list
     *            of all TEs of a given type in the event. The same type may appear multiple times
     *            in the case of identical input particle types.
     * @param nav pointer to the navigation service.
     */
    ComboIterator(std::vector<TEVec> tes, const TrigNavStructure* nav);

    /** @brief Rewind method, resetting the iterator to the first element. */
    bool rewind();

    /** @brief Unary * operator, used to recover the current combination. */
    TEVec& operator*() { return combination(); }

    /** @brief Validity check for the iterator.
     *  @return result of the validity check; returns false if iterator is at end, true otherwise.
     */
    bool isValid() const { return m_valid; }

    /** @brief Accessor method for the current combination. */
    TEVec& combination() { return m_comb; }

    /** @brief Post increment operator. */
    ComboIterator& operator++(int) { return operator++(); }

    /** @brief Pre increment operator. */
    ComboIterator& operator++();
   
  private:

    /** @brief Method used to test overlaps between two TEs.
     *  @return result of the overlap test; returns true if the two TEs correspond to the same RoI, false otherwise.
     *  @param t1 first TE to check.
     *  @param t2 second TE to check.
     */
    bool overlaps(const TriggerElement* t1, const TriggerElement* t2) const;

    /** @brief Method used to test overlaps between a TE and a vector of TEs.
     *  @return result of the overlap test; returns true if the TE corresponds to the same RoI as one of the TEs in the vector, false otherwise.
     *  @param t1 TE to check.
     *  @param teVec TE vector to check.
     *  @param idx optional integer parameter, limiting the check over the 0..idx-1 range of the vector.
     */
    bool overlaps(const TriggerElement* t1, 
		  const TEVec& teVec, int idx = -1) const;

    /** @brief Private increment method. */
    bool incrementByOne(int pos, bool& ok);

    /** @brief Private increment method. */
    bool increment     (int pos);

    /** @brief Private reset method. */
    bool     reset     (int pos);

    /** @brief Method used to invalidate the current combination. */
    void invalidate();

    /** @brief Debug dump to std::cout. */
    void print() const;

    /** @brief Current combination of TEs. */
    TEVec m_comb;

    /** @brief Vector of indexes keeping track of the loop over combinations. */
    std::vector<int> m_idx;

    /** @brief Vector of vectors of TEs to be combined. */
    std::vector<TEVec> m_tes;

    /** @brief Validity status variable. */
    bool m_valid;

    /** @brief Pointer to the navigation service. */
    const TrigNavStructure* m_nav;
  };


  /**
   * @brief Iterator used to loop over multi-particle combinations already used to seed a set
   * topological TEs
   *
   * @author Carlo Schiavi  <Carlo.Schiavi@ge.infn.it>  -  INFN Genova
   *
   * This iterator class is used by HLT algorithms to iterate over multi-particle combinations.
   * Given a set of topological TEs, passed in the constructor, the iterator can be used to move
   * over all the combinations of TEs originating from those already used to seed them. These
   * combinations are built taking the predecessors of the topological objects and then navigating
   * from each of them, reaching the TE types specified in the constructor.
   */

  class ComboIteratorTopological : public ComboIteratorBase {

  public:
    /**
     * @brief Constructor; gets a vector of topological TEs to start from, a vector of types with
     * which combinations must be formed and a pointer to Navigation as arguments.
     * @param topologicalTEs vector of topological TEs to start from.
     * @param teTypes vector of TE types by which combinations must be formed.
     * @param nav pointer to the navigation service.
     */
    ComboIteratorTopological(TEVec topologicalTEs, std::vector<unsigned int> teTypes, const TrigNavStructure* nav, bool onlyActive=true);

    /** @brief Rewind method, resetting the iterator to the first element. */
    bool rewind();

    /** @brief Unary * operator, used to recover the current combination. */
    TEVec& operator*() { return combination(); }

    /** @brief Validity check for the iterator.
     *  @return result of the validity check; returns false if iterator is at end, true otherwise.
     */
    bool isValid() const { return m_valid; }

    /** @brief Accessor method for the current combination. */
    TEVec& combination() { return (*(m_combination)); }

    /** @brief Post increment operator. */
    ComboIteratorTopological& operator++(int) { return operator++(); }

    /** @brief Pre increment operator. */
    ComboIteratorTopological& operator++();
   
  private:

    /** @brief Method used to invalidate the current combination. */
    void invalidate() { m_valid=false; }

    /** @brief Debug dump to std::cout. */
    void print() const {/**/};

    /** @brief Iterator pointing to the current combination. */
    std::vector<TEVec>::iterator m_combination;

    /** @brief Vector of vectors corresponding to the available combinations. */
    std::vector<TEVec> m_combinations;

    /** @brief Validity status variable. */
    bool m_valid;

    /** @brief Pointer to the navigation service. */
    const TrigNavStructure* m_nav;
  };

} // eof namespace

#endif //#ifndef
