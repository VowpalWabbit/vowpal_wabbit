#include "arguments.h"

bool operator==(const base_argument& lhs, const base_argument& rhs) {
  return lhs.m_name == rhs.m_name
    && lhs.m_type_hash == rhs.m_type_hash
    && lhs.m_help == rhs.m_help
    && lhs.m_short_name == rhs.m_short_name
    && lhs.m_keep == rhs.m_keep;
}

bool operator!=(const base_argument& lhs, const base_argument& rhs) {
  return !(lhs == rhs);
}
