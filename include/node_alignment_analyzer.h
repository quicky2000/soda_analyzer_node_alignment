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
#ifndef _NODE_ALIGNMENT_ANALYZER_H_
#define _NODE_ALIGNMENT_ANALYZER_H_

#include "cpp_analyzer_base.h"
#include "node_alignment_analyzer_description.h"
#include "node_alignment_analyzer_common_api.h"
#include "module_configuration.h"
#include "changeset.h"

#include <inttypes.h>
#include <map>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <set>
#include <iomanip>

namespace osm_diff_analyzer_node_alignment
{
  class node_alignment_common_api;

  class node_alignment_analyzer:public osm_diff_analyzer_cpp_if::cpp_analyzer_base
  {
  public:
    node_alignment_analyzer(const osm_diff_analyzer_if::module_configuration * p_conf,
                            node_alignment_analyzer_common_api & p_api);
    ~node_alignment_analyzer(void);
    // Methods inherited from cpp_analyzer_if
    void init(const osm_diff_analyzer_if::osm_diff_state * p_diff_state);
    void analyze(const std::vector<osm_api_data_types::osm_change*> & p_changes);
    const std::string & get_input_type(void)const;
    const std::string & get_output_type(void)const;
    const std::string & get_type(void)const;
    // End of inherited methods
  private:
    template <class T>
      void generic_analyze(const osm_api_data_types::osm_core_element * const p_object);

    node_alignment_analyzer_common_api & m_api;
    std::ofstream m_report;
    std::map<osm_api_data_types::osm_object::t_osm_id,changeset *> m_changesets;
    std::set<osm_api_data_types::osm_object::t_osm_id> m_encountered_changesets;
    static node_alignment_analyzer_description m_description;
  };
  //------------------------------------------------------------------------------
  template <class T>
    void node_alignment_analyzer::generic_analyze(const osm_api_data_types::osm_core_element * const p_object)
  {

    const T * const l_casted_object = dynamic_cast<const T * const>(p_object);
    if(l_casted_object==NULL)
      {
        std::cout << "ERROR : invalid " << T::get_type_str() << " cast for object id " << p_object->get_id() << std::endl ;
        exit(-1);
      }

    // Extract changeset
    osm_api_data_types::osm_object::t_osm_id l_changeset_id = l_casted_object->get_changeset();
    osm_api_data_types::osm_object::t_osm_id l_user_id = l_casted_object->get_user_id();
    const std::string & l_user_name = l_casted_object->get_user();
    
    // Mark changeset as encountered
    m_encountered_changesets.insert(l_changeset_id);
    std::map<osm_api_data_types::osm_object::t_osm_id,changeset *>::iterator l_changeset_iter = m_changesets.find(l_changeset_id);
    if(l_changeset_iter == m_changesets.end())
      {
        std::cout << "Create changeset " << l_changeset_id << std::endl ;
        l_changeset_iter = m_changesets.insert(std::map<osm_api_data_types::osm_object::t_osm_id,changeset *>::value_type(l_changeset_id,new changeset(m_report,l_changeset_id,l_user_name,l_user_id))).first;
      }
    l_changeset_iter->second->add(*l_casted_object);
  }
}
#endif
