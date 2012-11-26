/*
  This file is part of osm_diff_analyzer_node_alignment, Openstreetmap
  diff analyzer based on CPP diff representation. It's aim is to survey
  ways edited and to generate an alert in case of node alignment
  Copyright (C) 2012  Julien Thevenon ( julien_thevenon at yahoo.fr )

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include <vector>

#ifndef _LINEAR_REGRESSION_H_
#define _LINEAR_REGRESSION_H_
namespace osm_diff_analyzer_node_alignment
{
  class linear_regression
  {
  public:
    inline linear_regression(void);
    inline double compute(const std::vector<std::pair<double,double> > & p_list);
    inline const double & get_max_alignment_square(void)const;
    inline const double & get_average_x(void)const;
    inline const double & get_average_y(void)const;
  private:
    double m_max_alignment_square;
    double m_average_x;
    double m_average_y;
    double m_a;
    double m_b;
  };

  //----------------------------------------------------------------------------
  const double & linear_regression::get_max_alignment_square(void)const
    {
      return m_max_alignment_square;
    }

  //----------------------------------------------------------------------------
  const double & linear_regression::get_average_x(void)const
    {
      return m_average_x;
    }

  //----------------------------------------------------------------------------
  const double & linear_regression::get_average_y(void)const
    {
      return m_average_y;
    }

  //----------------------------------------------------------------------------
  linear_regression::linear_regression(void):
    m_max_alignment_square(0.0),
    m_average_x(0.0),
    m_average_y(0.0),
    m_a(0.0),
    m_b(0.0)
      {
      }
    
    //----------------------------------------------------------------------------
    double linear_regression::compute(const std::vector<std::pair<double,double> > & p_list)
    {
      m_max_alignment_square = 0;
      double l_sum = 0.0;
      m_average_x = 0.0;
      m_average_y = 0.0;
      for(std::vector<std::pair<double,double> >::const_iterator l_iter = p_list.begin();
          l_iter != p_list.end();
          ++l_iter)
        {
          m_average_x += l_iter->first;
          m_average_y += l_iter->second;
        }
      m_average_x = m_average_x / p_list.size(); 
      m_average_y = m_average_y / p_list.size(); 
  
      // a computation
      m_a = 0;
      double l_num = 0;
      double l_den = 0;
      for(std::vector<std::pair<double,double> >::const_iterator l_iter = p_list.begin();
          l_iter != p_list.end();
          ++l_iter)
        {
          l_num += (l_iter->first - m_average_x) * (l_iter->second - m_average_x);
          l_den += (l_iter->first - m_average_x) * (l_iter->first - m_average_x);
        }  
      if(l_den)
        {
          m_a = l_num / l_den;
          m_b = m_average_y - m_a * m_average_x;
          for(std::vector<std::pair<double,double> >::const_iterator l_iter = p_list.begin();
              l_iter != p_list.end();
              ++l_iter)
            {
              double l_diff = (l_iter->second - m_a * l_iter->first - m_b);
              double l_diff_square = l_diff * l_diff;
              if(m_max_alignment_square < l_diff_square)
                {
                  m_max_alignment_square = l_diff_square;
                }
              l_sum += l_diff_square; 
            }      
        }
      else
        {
          m_a = l_den / l_num;
          double m_b = m_average_x - m_a * m_average_y;
          for(std::vector<std::pair<double,double> >::const_iterator l_iter = p_list.begin();
              l_iter != p_list.end();
              ++l_iter)
            {
              double l_diff = (l_iter->first - m_a * l_iter->second - m_b);
              double l_diff_square = l_diff * l_diff;
              if(m_max_alignment_square < l_diff_square)
                {
                  m_max_alignment_square = l_diff_square;
                }
              l_sum += l_diff_square;
            }      
        }
      return l_sum;
    }

}
#endif // _LINEAR_REGRESSION_H_
//EOF
