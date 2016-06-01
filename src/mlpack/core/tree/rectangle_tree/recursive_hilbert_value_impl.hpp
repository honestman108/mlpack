/**
 * @file recursive_hilbert_value_impl.hpp
 * @author Mikhail Lozhnikov
 *
 * Implementation of the RecursiveHilbertValue class, a class that measures
 * ordering of points recursively.
 */
#ifndef MLPACK_CORE_TREE_RECTANGLE_TREE_RECURSIVE_HILBERT_VALUE_IMPL_HPP
#define MLPACK_CORE_TREE_RECTANGLE_TREE_RECURSIVE_HILBERT_VALUE_IMPL_HPP

namespace mlpack {
namespace tree /** Trees and tree-building procedures. */ {


template<typename ElemType>
int RecursiveHilbertValue::ComparePoints(const arma::Col<ElemType> &pt1,
                                         const arma::Col<ElemType> &pt2)
{
  size_t dim = pt1.n_rows;
  CompareStruct<ElemType> comp(dim);

  return ComparePoints(pt1,pt2,comp);
};

template<typename TreeType>
int RecursiveHilbertValue::CompareValues(TreeType *tree,
                       RecursiveHilbertValue &val1, RecursiveHilbertValue &val2)
{
  typedef typename TreeType::ElemType ElemType;
  size_t point1 = val1.LargestValue();
  size_t point2 = val2.LargestValue();

  return ComparePoints(arma::Col<ElemType>(tree->Dataset().col(point1)),
                       arma::Col<ElemType>(tree->Dataset().col(point2)));
}

template<typename TreeType>
int RecursiveHilbertValue::CompareWith(TreeType *tree,
                                       RecursiveHilbertValue &val)
{
  return CompareValues(tree,*this,val);
}

template<typename TreeType,typename ElemType>
int RecursiveHilbertValue::CompareWith(TreeType *tree,
                                       const arma::Col<ElemType> &pt)
{
  return ComparePoints(arma::Col<ElemType>(tree->Dataset()->col(largestValue)),pt);
}

template<typename TreeType>
int RecursiveHilbertValue::CompareWith(TreeType *tree,
                                       const size_t point)
{
  typedef typename TreeType::ElemType ElemType;
  return ComparePoints(arma::Col<ElemType>(tree->Dataset().col(largestValue)),
                       arma::Col<ElemType>(tree->Dataset().col(point)));
}


template<typename ElemType>
int RecursiveHilbertValue::ComparePoints(const arma::Col<ElemType> &pt1,
                                         const arma::Col<ElemType> &pt2,
                                         CompareStruct<ElemType> &comp)
{
  arma::Col<ElemType> center = comp.Hi * 0.5;
  arma::Col<ElemType> vec = comp.Lo * 0.5;
  std::vector<int> bits(pt1.n_rows,0);
  std::vector<int> bits2(pt1.n_rows,0);

  center += vec;

  // Get bits in order to use the Gray code
  for(size_t i = 0; i < pt1.n_rows; i++)
  {
    size_t j = comp.permutation[i];
    bits[i] = (pt1(j) > center(j) && !comp.inversion[j]) ||
       (pt1(j) <= center(j) && !comp.inversion[j]);

    bits2[i] = (pt2(j) > center(j) && !comp.inversion[j]) ||
       (pt2(j) <= center(j) && !comp.inversion[j]);
  }

  // Gray encode
  for(size_t i = 1; i < pt1.n_rows; i++)
  {
    bits[i] ^= bits[i-1];
    bits2[i] ^= bits2[i-1];
  }

  if(comp.invertResult)
  {
    for(size_t i = 0; i < pt1.n_rows; i++)
    {
      bits[i] = !bits[i];
      bits2[i] = !bits2[i];
    }
  }

  for(size_t i = 0; i < pt1.n_rows; i++)
  {
    if(bits[i] < bits2[i])
      return -1;
    if(bits[i] > bits2[i])
      return 1;
  }

  if(comp.recursionLevel >= recursionDepth)
    return 0;

  comp.recursionLevel++;

  if(bits[pt1.n_rows-1])
    comp.invertResult = !comp.invertResult;

  // Since the Hilbert curve is continuous we should permutate and intend
  // coordinate axes depending on the position of the point
  for(size_t i = 0; i < pt1.n_rows; i++)
  {
    size_t j = comp.permutation[i];
    size_t j0 = comp.permutation[0];
    if((pt1(j) > center(j) && !comp.inversion[j]) ||
       (pt1(j) <= center(j) && !comp.inversion[j]))
      comp.inversion[j0] = !comp.inversion[j0];
    else
    {
      size_t tmp;
      tmp = comp.permutation[0];
      comp.permutation[0] = comp.permutation[i];
      comp.permutation[i] = tmp;
    }      
  }

  // Choose an appropriate subhypercube
  for(size_t i = 0; i < pt1.n_rows; i++)
  {
    if(pt1(i) > center(i))
      comp.Lo(i) = center(i);
    else
      comp.Hi(i) = center(i);
  }

  return ComparePoints(pt1,pt2,comp);
}

template<typename TreeType>
size_t RecursiveHilbertValue::InsertPoint(TreeType *node, const size_t point)
{
  typedef typename TreeType::ElemType ElemType;
  if(node->IsLeaf())
  {
    size_t i;

    for(i = 0; i < node->NumPoints(); i++)
      if(ComparePoints(arma::Col<ElemType>(node->LocalDataset().col(i)),
                       arma::Col<ElemType>(node->Dataset().col(point)))> 0)
        break;
    if(i == node->NumPoints())
      largestValue = point;

    return i;
  }
  else
  {
    if(largestValue < 0)
    {
      largestValue = point;
      return 0;
    }
    if(ComparePoints(arma::Col<ElemType>(node->Dataset().col(point)),
                     arma::Col<ElemType>(node->Dataset().col(largestValue))) > 0)
      largestValue = point;
  }
  return 0;
}

template<typename TreeType>
void RecursiveHilbertValue::InsertNode(TreeType *node)
{
  typedef typename TreeType::ElemType ElemType;
  size_t point = node->AuxiliaryInfo().LargestHilbertValue().LargestValue();

  if(ComparePoints(arma::Col<ElemType>(node->Dataset()->col(point)),
                   arma::Col<ElemType>(node->Dataset()->col(largestValue))) > 0)
    largestValue = point;
}

template<typename TreeType>
void RecursiveHilbertValue::DeletePoint(TreeType *node, const size_t localIndex)
{
  if(node->NumPoints() <= 1)
  {
    largestValue = -1;
    return;
  }
  if(localIndex + 1 == node->NumPoints())
    largestValue = node->Points()[localIndex-1];
  
}

template<typename TreeType>
void RecursiveHilbertValue::RemoveNode(TreeType *node, const size_t nodeIndex)
{
  if(node->NumChildren() <= 1)
  {
    largestValue = -1;
    return;
  }
  if(nodeIndex + 1 == node->NumChildren())
    largestValue = node->Children()[nodeIndex-1]->AuxiliaryInfo.LargestHilbertValue().LargestValue();

}

inline RecursiveHilbertValue RecursiveHilbertValue::operator = (const RecursiveHilbertValue &val)
{
  largestValue = val.LargestValue();

  return *this;
}

template<typename TreeType>
void RecursiveHilbertValue::Copy(TreeType *dst, TreeType *src)
{
  dst->AuxiliaryInfo().LargestHilbertValue().LargestValue() =
    src->AuxiliaryInfo().LargestHilbertValue().LargestValue();
}

template<typename TreeType>
void RecursiveHilbertValue::UpdateLargestValue(TreeType *node)
{
  if(node->IsLeaf())
  {
    largestValue = (node->NumPoints() > 0 ?
                    node->Points()[node->NumPoints() - 1] : -1);
  }
  else
  {
    largestValue = (node->NumChildren() > 0 ?
                    node->Children()[node->NumChildren() - 1]->AuxiliaryInfo().LargestHilbertValue().LargestValue() : -1);
  }
}


} // namespace tree
} // namespace mlpack

#endif //MLPACK_CORE_TREE_RECTANGLE_TREE_RECURSIVE_HILBERT_VALUE_IMPL_HPP
