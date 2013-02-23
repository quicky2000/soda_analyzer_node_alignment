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

#include "changeset.h"
#include "way.h"
#include "node.h"
#include "osm_way.h"
#include "svg_report.h"
#include "linear_regression.h"
#include "node_alignment_analyzer.h"
#include "quicky_exception.h"
#include <sstream>
#include <limits>
#include <cmath>
#include <iomanip>

namespace osm_diff_analyzer_node_alignment
{
  //----------------------------------------------------------------------------
  void changeset::add(const osm_api_data_types::osm_way & p_way)
  {

    // Create a simplified representation of way that will survive to diff end of life
    way * l_way = new way(p_way.get_id(),p_way.get_user(),p_way.get_user_id(),p_way.get_version(),true);
    m_ways.insert(std::map<osm_api_data_types::osm_object::t_osm_id,way*>::value_type(p_way.get_id(),l_way));
    const std::vector<osm_api_data_types::osm_object::t_osm_id> & l_node_refs = p_way.get_node_refs();
    l_way->set_node_refs(l_node_refs);
  }
  //----------------------------------------------------------------------------
  void changeset::add(const osm_api_data_types::osm_node & p_node)
  {
    node * l_node = new node(p_node.get_id(),p_node.get_user(),p_node.get_user_id(),p_node.get_version(),p_node.get_lat(),p_node.get_lon(),true);
    m_nodes.insert(std::map<osm_api_data_types::osm_object::t_osm_id,node*>::value_type(p_node.get_id(),l_node));
    m_nodes_to_check.insert(p_node.get_id());
  }

  //----------------------------------------------------------------------------
  changeset::~changeset(void)
  {
    for(std::map<osm_api_data_types::osm_object::t_osm_id,way*>::iterator l_iter = m_ways.begin();
        l_iter != m_ways.end();
        ++l_iter)
      {
        delete l_iter->second;
      }
    for(std::map<osm_api_data_types::osm_object::t_osm_id,node*>::iterator l_iter = m_nodes.begin();
        l_iter != m_nodes.end();
        ++l_iter)
      {
        delete l_iter->second;
      }
  }
  //----------------------------------------------------------------------------
  void changeset::search_aligned_ways(std::ofstream & p_stream)
  {
    // First check if modified ways has been aligned to eliminate a maximum of nodes to limite API
    // call that will be done later for each node to determine to which way it belongs
    // If a way has been aligned all its nodes will be removed and no more analyzed
    for(std::map<osm_api_data_types::osm_object::t_osm_id,way*>::iterator l_iter_way = m_ways.begin();
        l_iter_way != m_ways.end();
        ++l_iter_way)
      {
        check_way(l_iter_way->second->get_id(),l_iter_way->second->get_node_refs());
      }


    // For each node get ways
    while(m_nodes_to_check.size())
      {
        std::set<osm_api_data_types::osm_object::t_osm_id>::iterator l_iter_id = m_nodes_to_check.begin();
        std::map<osm_api_data_types::osm_object::t_osm_id,node*>::iterator l_iter_node = m_nodes.find(*l_iter_id);

        bool l_aligned = false;
        const std::vector<osm_api_data_types::osm_way*> * const l_ways = m_api->get_node_ways(l_iter_node->second->get_id());
        for(std::vector<osm_api_data_types::osm_way*>::const_iterator l_iter_way = l_ways->begin();
            l_iter_way != l_ways->end() && !l_aligned;
            ++l_iter_way)
          {
            if(!l_aligned && m_checked_ways.find((*l_iter_way)->get_id()) == m_checked_ways.end())
              {
                l_aligned = check_way((*l_iter_way)->get_id(),(*l_iter_way)->get_node_refs());
                m_checked_ways.insert((*l_iter_way)->get_id());
              }
          }
        for(std::vector<osm_api_data_types::osm_way*>::const_iterator l_iter_way = l_ways->begin();
            l_iter_way != l_ways->end();
            ++l_iter_way)
          {
            delete *l_iter_way;
          }
        delete l_ways;
        if(!l_aligned)
          {
            m_nodes_to_check.erase(l_iter_id);
          }

      }

  }

  //----------------------------------------------------------------------------
  bool changeset::check_way(const osm_api_data_types::osm_object::t_osm_id & p_id,const std::vector<osm_api_data_types::osm_object::t_osm_id> & p_node_refs)
  {
    bool l_result = false;
    if(p_node_refs.size()> m_min_way_node_nb)
      {
        std::vector<node*> l_modified_nodes;
        std::map<osm_api_data_types::osm_object::t_osm_id,std::pair<double,double> > m_old_nodes_coordinates;

        // Check if some existings nodes belong to this way
        for(std::vector<osm_api_data_types::osm_object::t_osm_id>::const_iterator l_way_node = p_node_refs.begin();
            l_way_node != p_node_refs.end();
            ++l_way_node)
          {
            std::map<osm_api_data_types::osm_object::t_osm_id,node*>::iterator l_node_iter = m_nodes.find(*l_way_node);
            if(l_node_iter != m_nodes.end())
              {
                l_modified_nodes.push_back(l_node_iter->second);
              }
          }
        // check if more than coef % node has been modified : an abusive alignment modify almost every node except one
        float l_modif_rate = ((float)(l_modified_nodes.size())/((float)p_node_refs.size()));
        if(l_modified_nodes.size() == p_node_refs.size() - 2 || l_modif_rate > m_modif_rate_min_level)
          {
            // Check how many nodes has been moved by comparing with previous version of node
            // The check stop if the number of unmoved node is sufficiant to be sure that the modification rate will not be reached
            uint32_t l_nb_moved_node = l_modified_nodes.size();
            l_modif_rate = ((float)(l_nb_moved_node)/((float)p_node_refs.size()));
            for(std::vector<node*>::const_iterator l_iter_node = l_modified_nodes.begin();
                l_iter_node != l_modified_nodes.end() && ( l_modif_rate > m_modif_rate_min_level || l_nb_moved_node >= p_node_refs.size() - 2 );
                ++l_iter_node)
              {
                const osm_api_data_types::osm_node * l_previous_node = m_api->get_node_version((*l_iter_node)->get_id(),(*l_iter_node)->get_version()-1);
                assert(l_previous_node);
                if(l_previous_node->get_lat() == (*l_iter_node)->get_lat() && l_previous_node->get_lon() == (*l_iter_node)->get_lon())
                  {
                    --l_nb_moved_node;
                    l_modif_rate = ((float)(l_nb_moved_node)/((float)p_node_refs.size()));
                  }
                else
                  {
                    m_old_nodes_coordinates.insert(std::map<osm_api_data_types::osm_object::t_osm_id,std::pair<double,double> >::value_type((*l_iter_node)->get_id(),std::pair<double,double>(l_previous_node->get_lat(),l_previous_node->get_lon())));
                  }
                delete l_previous_node;
              }
            // Check if verification has been completed : sign of complete aligned way
            if(l_modif_rate > m_modif_rate_min_level || l_nb_moved_node >= p_node_refs.size() - 2 )
              {

                //Reconstitute ways
                std::vector<std::pair<double,double> > l_old_coordinates2;
                std::vector<std::pair<double,double> > l_new_coordinates2;
                for(std::vector<osm_api_data_types::osm_object::t_osm_id>::const_iterator l_way_node = p_node_refs.begin();
                    l_way_node != p_node_refs.end();
                    ++l_way_node)
                  {
                    std::pair<double,double> l_current_coordinates;
                    std::map<osm_api_data_types::osm_object::t_osm_id,node*>::iterator l_node_iter = m_nodes.find(*l_way_node);
                    bool l_bad_coordinates = false;
                    if(l_node_iter != m_nodes.end())
                      {
                        l_current_coordinates = std::pair<double,double>(l_node_iter->second->get_lat(),l_node_iter->second->get_lon());
                      }
                    else 
                      {
                        const osm_api_data_types::osm_node * l_node = m_api->get_node_version(*l_way_node);
                        if(l_node != NULL)
                          {
                            l_current_coordinates = std::pair<double,double>(l_node->get_lat(),l_node->get_lon());
                            delete l_node;
                          }
                        else
                          {
                            l_bad_coordinates = true;
                          }
                      }
                    
                    if(!l_bad_coordinates)
                      {
                        l_new_coordinates2.push_back(l_current_coordinates);
                      }

                    std::map<osm_api_data_types::osm_object::t_osm_id,std::pair<double,double> >::const_iterator l_iter_coordinates = m_old_nodes_coordinates.find(*l_way_node);
                    if(l_iter_coordinates != m_old_nodes_coordinates.end())
                      {
                        l_old_coordinates2.push_back(l_iter_coordinates->second);
                      }
                    else if(!l_bad_coordinates)
                      {
                        l_old_coordinates2.push_back(l_current_coordinates);
                      }
                  }


                linear_regression l_regress_old;
                double l_old_result = l_regress_old.compute(l_old_coordinates2);
                linear_regression l_regress_new;
                double l_new_result = l_regress_new.compute(l_new_coordinates2);

                double l_alignment_modification_rate = ( l_new_result ? l_old_result / l_new_result : std::numeric_limits<double>::max());
                double l_old_max_diff_square = l_regress_old.get_max_alignment_square();
                double l_new_max_diff_square = l_regress_new.get_max_alignment_square();
                double l_min_square_modification_rate = ( l_new_max_diff_square ? l_old_max_diff_square / l_new_max_diff_square : std::numeric_limits<double>::max());

                // Way has been aligned, remove node form analyzis queue to reduce API requests
                if(l_alignment_modification_rate > m_min_alignment_modification_rate && l_min_square_modification_rate  > m_min_alignment_modification_rate)
                  {
		    create_svg(p_id,l_old_coordinates2,l_new_coordinates2);
                    l_result = true;
                    std::stringstream l_id_stream;
                    l_id_stream << m_id;
                    std::stringstream l_way_id_stream;
                    l_way_id_stream << p_id;

                    std::string l_old_gpx = "way_"+l_way_id_stream.str()+"_c"+l_id_stream.str()+"_old";
                    create_gpx(l_old_gpx,l_old_coordinates2);
                    std::string l_new_gpx = "way_"+l_way_id_stream.str()+"_c"+l_id_stream.str()+"_new";
                    create_gpx(l_new_gpx,l_new_coordinates2);

                    std::string l_object_url;
                    m_api->get_object_browse_url(l_object_url,"way",p_id); 
                    std::string l_changeset_url;
                    m_api->get_object_browse_url(l_changeset_url,"changeset",m_id);
                    std::string l_user_url;
                    m_api->get_user_browse_url(l_user_url,m_user_id,m_user_name);
                    if(!m_report.is_open())
                      {
                        m_analyzer.create_report();
                      }
                    m_report << "<A HREF=\"" << l_object_url << "\">Way " << l_way_id_stream.str() << "</A> has been aligned by <A HREF=\"" << l_user_url << "\">" << m_user_name << "</A> in <A HREF=\"" << l_changeset_url << "\">Changeset " << l_id_stream.str() << "</A><BR>" << std::endl ;
                    m_report << "With <B>alignment modification rate = " << l_alignment_modification_rate << "</B> and <B>Min square modification rate = " << l_min_square_modification_rate << "</B><BR>"  << std::endl ;
                    std::string l_map_name = "map_"+l_way_id_stream.str()+"_c"+l_id_stream.str();
                    m_report << "<button type=\"button\" onclick=\"init('" << l_map_name << "','" << l_old_gpx << ".gpx','" << l_new_gpx << ".gpx'," << l_regress_new.get_average_x() << "," << l_regress_new.get_average_y() << ")\">Display Map</button>" << std::endl;
                    m_report << "<div id=\"" << l_map_name << "\" class=\"smallmap\">" <<std::endl ;
                    m_report << "</div>" << std::endl ;
                    m_report << "<IMG SRC=\"./way_" << l_way_id_stream.str() << ".svg\" ALT=\"Way_" << l_way_id_stream.str() << ".svg\" TITLE=\"Way " << l_way_id_stream.str() << "\" ALIGN=\"MIDDLE\" /><BR>" << std::endl ;
                    m_report << "<HR/>" << std::endl ;
                  for(std::vector<node*>::iterator l_iter = l_modified_nodes.begin();
                        l_iter != l_modified_nodes.end();
                        ++l_iter)
                      {
                        m_nodes_to_check.erase((*l_iter)->get_id());
                      }
                  }
              }
          }
      }
    return l_result;
  }

  //----------------------------------------------------------------------------
  void changeset::create_gpx(const std::string & p_way_name,
                             const std::vector<std::pair<double,double> > & p_points)
  {
    std::string l_file_name = p_way_name + ".gpx";
    std::ofstream l_gpx_file(l_file_name.c_str());
    if(!l_gpx_file.is_open())
      {
	std::stringstream l_stream;
	l_stream << "Error when creating GPX file \"" << l_file_name << "\"" ;
	throw quicky_exception::quicky_runtime_exception(l_stream.str(),__LINE__,__FILE__);
      }
    
    l_gpx_file << "<?xml version=\"1.0\"?>" << std::endl ;
    l_gpx_file << "<gpx version=\"1.0\""  << std::endl ;
    l_gpx_file << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""  << std::endl ;
    l_gpx_file << "xmlns=\"http://www.topografix.com/GPX/1/0\""  << std::endl ;
    l_gpx_file << "xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">"  << std::endl ;
    l_gpx_file << "<trk>" << std::endl ;
    l_gpx_file << "<name>" << p_way_name << "</name>" << std::endl ;
    l_gpx_file << "<trkseg>" << std::endl ;
    for(std::vector<std::pair<double,double> >::const_iterator l_iter = p_points.begin();
        l_iter != p_points.end();
        ++l_iter)
      {
        l_gpx_file << "<trkpt lat=\""<< std::setprecision(15) << l_iter->first << "\" lon=\"" << l_iter->second << "\">" << std::endl ;
        l_gpx_file << "</trkpt>" << std::endl ;
     }
    l_gpx_file << "</trkseg>" << std::endl ;
    l_gpx_file << "</trk>" << std::endl ;
    l_gpx_file << "</gpx>" << std::endl ;
    l_gpx_file.close();
  }

  void changeset::create_svg(const osm_api_data_types::osm_object::t_osm_id & p_id,
                             const std::vector<std::pair<double,double> > & p_old_list,
                             const std::vector<std::pair<double,double> > & p_new_list)
  {
    std::stringstream l_id_stream;
    l_id_stream << p_id;
    std::string l_file_name = "way_"+l_id_stream.str()+".svg";

    svg_report l_svg_report;
    l_svg_report.update_xtrem_coordinates(p_old_list);
    l_svg_report.update_xtrem_coordinates(p_new_list);
    l_svg_report.adjust_xtrem_coordinates(0.1);
  
    l_svg_report.open(l_file_name);
    l_svg_report.draw_polyline(p_old_list,"blue",1);
    l_svg_report.draw_polyline(p_new_list,"red",0);
    l_svg_report.close();

  }


  float changeset::m_modif_rate_min_level = 0.9;
  float changeset::m_min_alignment_modification_rate = 100;
  node_alignment_common_api * changeset::m_api = NULL;
  uint32_t changeset::m_min_way_node_nb = 2;
}
//EOF
