    // Prefix increment
    IntervalIterator &operator++ ()
    {
      if (m_current_sub_index <= m_parent->m_intervals.size())
        {
          ++m_current_sub_index;
          return m_parent->m_intervals.at(m_current_sub_index);
        }
      return *this;
    }