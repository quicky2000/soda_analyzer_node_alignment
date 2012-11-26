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
#include "node_alignment_analyzer.h"
#include "node_alignment_analyzer_common_api.h"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cassert>

namespace osm_diff_analyzer_node_alignment
{
  //------------------------------------------------------------------------------
  node_alignment_analyzer::node_alignment_analyzer(const osm_diff_analyzer_if::module_configuration * p_conf,
                                                   node_alignment_analyzer_common_api & p_api):
    osm_diff_analyzer_cpp_if::cpp_analyzer_base("node_alignment_analyser",p_conf->get_name(),""),
    m_api(p_api),
    m_report()
  {
    const std::map<std::string,std::string> & l_conf_parameters = p_conf->get_parameters();

    std::map<std::string,std::string>::const_iterator l_iter = l_conf_parameters.find("min_way_node_nb");
    if(l_iter == l_conf_parameters.end())
    {
      std::cout << this->get_name() << " : Using default value for parameter \"min_way_node_nb\" : " << changeset::get_min_way_node_nb() << std::endl ;
    }
    else
      {
	float l_min_way_node_nb = strtof(l_iter->second.c_str(),NULL);
	std::cout << this->get_name() << " : Using value " << l_min_way_node_nb << " for parameter \"min_way_node_nb\"" << std::endl ;
	changeset::set_min_way_node_nb(l_min_way_node_nb);
      }
    l_iter = l_conf_parameters.find("modif_rate_min_level");
    if(l_iter == l_conf_parameters.end())
    {
      std::cout << this->get_name() << " : Using default value for parameter \"modif_rate_min_level\" : " << changeset::get_modif_rate_min_level() << std::endl ;
    }
    else
      {
	float l_modif_rate_min_level = strtof(l_iter->second.c_str(),NULL);
	std::cout << this->get_name() << " : Using value " << l_modif_rate_min_level << " for parameter \"modif_rate_min_level\"" << std::endl ;
	changeset::set_modif_rate_min_level(l_modif_rate_min_level);
      }

    l_iter = l_conf_parameters.find("min_alignment_modification_rate");
    if(l_iter == l_conf_parameters.end())
    {
      std::cout << this->get_name() << " : Using default value for parameter \"min_alignment_modification_rate\" : " << changeset::get_min_alignment_modification_rate() << std::endl ;
    }
    else
      {
	float l_min_alignment_modification_rate = strtof(l_iter->second.c_str(),NULL);
	std::cout << this->get_name() << " : Using value " << l_min_alignment_modification_rate << " for parameter \"min_alignment_modification_rate\"" << std::endl ;
	changeset::set_min_alignment_modification_rate(l_min_alignment_modification_rate);
      }

    changeset::set_api(m_api);

    std::string l_report_file_name = this->get_name()+"_node_alignment_report";
    std::string l_complete_report_file_name = l_report_file_name + ".html";
    std::ifstream l_test_file;
    uint32_t l_number = 0;
    bool l_continu = true;
    while(l_continu)
      {
        l_test_file.open(l_complete_report_file_name.c_str());
        l_continu = l_test_file.is_open();
        if(l_continu)
          {
            ++l_number;
            std::stringstream l_number_str;
            l_number_str << l_number;
            l_complete_report_file_name = l_report_file_name + "_" + l_number_str.str() + ".html";
            l_test_file.close();
          }
      }

    
    // Creating report
    m_report.open(l_complete_report_file_name.c_str());
    if(m_report.fail())
      {
	std::cout << "ERROR : unabled to open \"" << l_complete_report_file_name << "\"" << std::endl ;
	exit(EXIT_FAILURE);
      }
    m_report << "<html>" << std::endl ;
    m_report << "\t<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">" << std::endl ;
    m_report << "\t\t<title>" << this->get_name() << " Node Alignement Report </title>" << std::endl ;

    m_report << "<script src=\"http://openlayers.org/api/OpenLayers.js\"></script>" << std::endl ;
    m_report << "<script type=\"text/javascript\">" << std::endl ;
    //    m_report << "var lat=34.070;" << std::endl ;
    //    m_report << "var lon=-118.73;" << std::endl ;
    m_report << "var zoom=15;" << std::endl ;
    m_report << "var map;" << std::endl ;
    m_report << std::endl ;
    m_report << "function init(name,old_file_name,new_file_name,lat,lon){" << std::endl ;
    m_report << "  var my_map = document.getElementById(name)" << std::endl ;
    m_report << "    my_map = new OpenLayers.Map (name, {" << std::endl ;
    m_report << "      controls:[" << std::endl ;
    m_report << "                new OpenLayers.Control.Navigation()," << std::endl ;
    m_report << "                new OpenLayers.Control.PanZoomBar()," << std::endl ;
    m_report << "                new OpenLayers.Control.LayerSwitcher()," << std::endl ;
    m_report << "                new OpenLayers.Control.Attribution()]," << std::endl ;
    m_report << "                                     maxExtent: new OpenLayers.Bounds(-20037508.34,-20037508.34,20037508.34,20037508.34)," << std::endl ;
    m_report << "                                     maxResolution: 156543.0399," << std::endl ;
    m_report << "                                     numZoomLevels: 19," << std::endl ;
    m_report << "                                     units: 'm'," << std::endl ;
    m_report << "                                     projection: new OpenLayers.Projection(\"EPSG:900913\")," << std::endl ;
    m_report << "                                     displayProjection: new OpenLayers.Projection(\"EPSG:4326\")" << std::endl ;
    m_report << "                                     } );" << std::endl ;
    m_report << std::endl ;
    m_report << "  my_map.addLayer(new OpenLayers.Layer.OSM());" << std::endl ;
    //   m_report << "  my_map.size = new OpenLayers.Size(300,200);" << std::endl ;
    m_report << "  var lonLat = new OpenLayers.LonLat(lon, lat).transform(new OpenLayers.Projection(\"EPSG:4326\"), new OpenLayers.Projection(\"EPSG:900913\"));" << std::endl ;
    m_report << std::endl ;
    m_report << "  my_map.setCenter (lonLat, zoom);" << std::endl ;
    m_report << std::endl ;
    m_report << "  //Initialise the vector layer using OpenLayers.Format.OSM" << std::endl ;
    m_report << "  var layer = new OpenLayers.Layer.Vector(old_file_name, {" << std::endl ;
    m_report << "    strategies: [new OpenLayers.Strategy.Fixed()]," << std::endl ;
    m_report << "                                              protocol: new OpenLayers.Protocol.HTTP({" << std::endl ;
    m_report << "                                                url: old_file_name,   //<-- relative or absolute URL to your .osm file" << std::endl ;
    m_report << "                                                    format: new OpenLayers.Format.GPX()" << std::endl ;
    m_report << "                                                    })," << std::endl ;
    m_report << "                                              style: {strokeColor: \"blue\", strokeWidth: 6, strokeOpacity: 1}," << std::endl ;
    m_report << "                                              projection: new OpenLayers.Projection(\"EPSG:4326\")" << std::endl ;
    m_report << "                                              });" << std::endl ;
    m_report << std::endl ;
    m_report << "  my_map.addLayers([layer]);" << std::endl ;
    m_report << std::endl ;
    m_report << "  //Initialise the vector layer using OpenLayers.Format.OSM" << std::endl ;
    m_report << "  var layer2 = new OpenLayers.Layer.Vector(new_file_name, {" << std::endl ;
    m_report << "    strategies: [new OpenLayers.Strategy.Fixed()]," << std::endl ;
    m_report << "                                               protocol: new OpenLayers.Protocol.HTTP({" << std::endl ;
    m_report << "                                                 url: new_file_name,   //<-- relative or absolute URL to your .osm file" << std::endl ;
    m_report << "                                                     format: new OpenLayers.Format.GPX()" << std::endl ;
    m_report << "                                                     })," << std::endl ;
    m_report << "                                               style: {strokeColor: \"red\", strokeWidth: 6, strokeOpacity: 1}," << std::endl ;
    m_report << "                                               projection: new OpenLayers.Projection(\"EPSG:4326\")" << std::endl ;
    m_report << "                                               });" << std::endl ;
    m_report << std::endl ;
    m_report << "  my_map.addLayers([layer2]);" << std::endl ;
    m_report << std::endl ;
    m_report << "}" << std::endl ;
    m_report << "</script>" << std::endl ;

    m_report << "\t</head>" << std::endl ;
    m_report << "\t<body><H1>" << this->get_name() << " Node alignement Report</H1>" << std::endl ;
  }

  //------------------------------------------------------------------------------
  node_alignment_analyzer::~node_alignment_analyzer(void)
  {

    for(std::map<osm_api_data_types::osm_object::t_osm_id,changeset *>::iterator l_iter = m_changesets.begin();
        l_iter != m_changesets.end();
        ++l_iter)
      {
        delete l_iter->second;
      }

    m_report << "</body>" << std::endl ;
    m_report << "</html>" << std::endl ;

    m_report.close();
  }

  //------------------------------------------------------------------------------
  void node_alignment_analyzer::init(const osm_diff_analyzer_if::osm_diff_state * p_diff_state)
  {
    std::vector<osm_api_data_types::osm_object::t_osm_id> l_closed_changesets;

    std::cout << get_name() << " : Starting analyze of diff " << p_diff_state->get_sequence_number() << std::endl ;

    // List closed changesets : IE changesets not mentionned in previous minute diffs
    for(std::map<osm_api_data_types::osm_object::t_osm_id,changeset *>::const_iterator l_iter = m_changesets.begin();
        l_iter != m_changesets.end();
        ++l_iter)
      {
        if(m_encountered_changesets.find(l_iter->first)==m_encountered_changesets.end())
          {
            l_closed_changesets.push_back(l_iter->first);
          }
      }
    // Close changesets
    for(std::vector<osm_api_data_types::osm_object::t_osm_id>::const_iterator l_iter = l_closed_changesets.begin();
        l_iter != l_closed_changesets.end();
        ++l_iter)
      {
        std::map<osm_api_data_types::osm_object::t_osm_id,changeset *>::iterator l_changeset_iter = m_changesets.find(*l_iter);
        assert(l_changeset_iter != m_changesets.end());
        l_changeset_iter->second->search_aligned_ways(m_report);
        delete l_changeset_iter->second;
        m_changesets.erase(l_changeset_iter);
      }

    //
    m_encountered_changesets.clear();
  }

  //------------------------------------------------------------------------------
  void node_alignment_analyzer::analyze(const std::vector<osm_api_data_types::osm_change*> & p_changes)
  {
    for(std::vector<osm_api_data_types::osm_change*>::const_iterator l_iter = p_changes.begin();
        l_iter != p_changes.end();
        ++l_iter)
      {

        if((*l_iter)->get_type() == osm_api_data_types::osm_change::MODIFICATION)
          {
            const osm_api_data_types::osm_core_element * const l_element = (*l_iter)->get_core_element();
            assert(l_element);
            switch(l_element->get_core_type())
              {
              case osm_api_data_types::osm_core_element::NODE :
                generic_analyze<osm_api_data_types::osm_node>(l_element);
                break;
              case osm_api_data_types::osm_core_element::WAY :
                generic_analyze<osm_api_data_types::osm_way>(l_element);
                break;
              case osm_api_data_types::osm_core_element::RELATION :
                break;
              case osm_api_data_types::osm_core_element::INTERNAL_INVALID:
                std::cout << "ERROR : unexpected core type value \"" << osm_api_data_types::osm_core_element::get_osm_type_str(l_element->get_core_type()) << "\"" << std::endl ;
                exit(-1);
                break;
              }
          }
      }
  }

  //------------------------------------------------------------------------------
  const std::string & node_alignment_analyzer::get_input_type(void)const
  {
    return m_description.get_input_type();
  }

  //------------------------------------------------------------------------------
  const std::string & node_alignment_analyzer::get_output_type(void)const
  {
    return m_description.get_output_type();
  }

  //------------------------------------------------------------------------------
  const std::string & node_alignment_analyzer::get_type(void)const
  {
    return m_description.get_type();
  }

  node_alignment_analyzer_description node_alignment_analyzer::m_description;
}
//EOF
