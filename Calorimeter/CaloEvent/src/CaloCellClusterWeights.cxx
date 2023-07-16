/* Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration */
#include "CaloEvent/CaloCellClusterWeights.h"

#include "CaloEvent/CaloCell.h"

#include <algorithm>

#ifndef CELLCLUSTERLOOKUP
#define CELLCLUSTERLOOKUP 200000
#endif

CaloCellClusterWeights::CaloCellClusterWeights(size_t size)
  : m_hashTable(size)
{
  this->clear(); 
}

CaloCellClusterWeights::CaloCellClusterWeights() 
  : CaloCellClusterWeights( CELLCLUSTERLOOKUP )
{ }

CaloCellClusterWeights::CaloCellClusterWeights(const CaloCellClusterWeights& cellClusterWeights)
  : m_hashTable(cellClusterWeights.m_hashTable)
{ }

CaloCellClusterWeights::~CaloCellClusterWeights()
= default;

const CaloCellClusterWeights::weight_t& CaloCellClusterWeights::operator[](size_t hash)   const 
{ return this->check(hash) ? std::get<1>(m_hashTable.at(hash)) : m_defaultValue; }
const CaloCellClusterWeights::weight_t& CaloCellClusterWeights::at(size_t hash)           const 
{ return this->check(hash) ? std::get<1>( m_hashTable.at(hash)) : m_defaultValue; }
const CaloCellClusterWeights::weight_t& CaloCellClusterWeights::at(const CaloCell* pCell) const 
{ return this->at(static_cast<size_t>(pCell->caloDDE()->calo_hash())); }

bool CaloCellClusterWeights::fastCheck(size_t hash)           const { return std::get<0>(m_hashTable.at(hash)); }
bool CaloCellClusterWeights::fastCheck(const CaloCell* pCell) const { return this->fastCheck(static_cast<size_t>(pCell->caloDDE()->calo_hash())); } 
bool CaloCellClusterWeights::check(size_t hash)               const { return hash < m_hashTable.size() && this->fastCheck(hash); }
bool CaloCellClusterWeights::check(const CaloCell* pCell)     const { return this->check(static_cast<size_t>(pCell->caloDDE()->calo_hash())); } 

void CaloCellClusterWeights::set(size_t hash,double value)           
{ 
  if ( hash < m_hashTable.size() ) {
    if ( this->fastCheck(hash) ) { 
      std::get<1>(m_hashTable[hash]).push_back(value); 
    } else {
      std::get<0>(m_hashTable[hash]) = true;
      std::get<1>(m_hashTable[hash]).push_back(value); 
    } 
  }
}

void CaloCellClusterWeights::set(const CaloCell* pCell,double value) { this->set(static_cast<size_t>(pCell->caloDDE()->calo_hash()),value); } 

void CaloCellClusterWeights::clear() { std::fill(m_hashTable.begin(),m_hashTable.end(),value_t(false,m_defaultValue)); }
void CaloCellClusterWeights::clear(size_t hash) { 
  if ( hash < m_hashTable.size() ) { std::get<0>(m_hashTable[hash]) = false; std::get<1>(m_hashTable[hash]) = m_defaultValue; } 
}
void CaloCellClusterWeights::reset() { this->clear(); }
