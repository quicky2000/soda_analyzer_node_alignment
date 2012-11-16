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
#include "node_alignment_analyzer_wrapper.h"
#include "node_alignment_analyzer_description.h"
#include "node_alignment_analyzer.h"
#include "node_alignment_analyzer_common_api.h"

#include <cassert>
#include <iostream>

namespace osm_diff_analyzer_node_alignment
{
  //----------------------------------------------------------------------------
  const char * node_alignment_analyzer_wrapper::get_api_version(void)
  {
    return MODULE_LIBRARY_IF_VERSION;
  }

  //----------------------------------------------------------------------------
  uint32_t node_alignment_analyzer_wrapper::get_api_size(void)
  {
    return MODULE_LIBRARY_IF_API_SIZE;
  }

  //----------------------------------------------------------------------------
  osm_diff_analyzer_if::analyzer_description_if * node_alignment_analyzer_wrapper::get_node_alignment_description(void)
  {
    return new node_alignment_analyzer_description();
  }

  //----------------------------------------------------------------------------
  osm_diff_analyzer_if::general_analyzer_if * node_alignment_analyzer_wrapper::create_node_alignment_analyzer(const osm_diff_analyzer_if::module_configuration * p_conf)
  {
    return new node_alignment_analyzer(p_conf,*m_common_api);
  }

  //----------------------------------------------------------------------------
  void node_alignment_analyzer_wrapper::require_common_api(osm_diff_analyzer_if::module_library_if::t_register_function p_func)
  {
    m_common_api = new node_alignment_analyzer_common_api(p_func);
  }

  //----------------------------------------------------------------------------
  void node_alignment_analyzer_wrapper::cleanup(void)
  {
    delete m_common_api;
  }
  node_alignment_analyzer_common_api * node_alignment_analyzer_wrapper::m_common_api = NULL;

  extern "C"
  {
    void register_module(uintptr_t* p_api,uint32_t p_api_size)
    {
      assert(p_api_size == MODULE_LIBRARY_IF_API_SIZE);
      std::cout << "Registration of node_alignment analyzer API " << std::endl ;
      p_api[osm_diff_analyzer_if::module_library_if::GET_API_VERSION] = (uintptr_t)node_alignment_analyzer_wrapper::get_api_version;
      p_api[osm_diff_analyzer_if::module_library_if::GET_API_SIZE] = (uintptr_t)node_alignment_analyzer_wrapper::get_api_size;
      p_api[osm_diff_analyzer_if::module_library_if::GET_DESCRIPTION] = (uintptr_t)node_alignment_analyzer_wrapper::get_node_alignment_description;
      p_api[osm_diff_analyzer_if::module_library_if::CREATE_ANALYZER] = (uintptr_t)node_alignment_analyzer_wrapper::create_node_alignment_analyzer;
      p_api[osm_diff_analyzer_if::module_library_if::REQUIRE_COMMON_API] = (uintptr_t)node_alignment_analyzer_wrapper::require_common_api;
      p_api[osm_diff_analyzer_if::module_library_if::CLEAN_UP] = (uintptr_t)node_alignment_analyzer_wrapper::cleanup;
    }
  }
}
//EOF
