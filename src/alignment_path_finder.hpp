
#ifndef RPVG_SRC_ALIGNMENTPATHFINDER_HPP
#define RPVG_SRC_ALIGNMENTPATHFINDER_HPP

#include <vector>
#include <map>

#include "vg/io/basic_stream.hpp"
#include "paths_index.hpp"
#include "alignment_path.hpp"

using namespace std;


template<class AlignmentType> 
class AlignmentPathFinder {

    public: 
    
       	AlignmentPathFinder(const PathsIndex & paths_index_in, const string library_type_in, const uint32_t max_pair_frag_length_in, const uint32_t min_mapq_filter_in, const double min_best_score_filter_in, const double max_softclip_filter_in);

		vector<AlignmentPath> findAlignmentPaths(const AlignmentType & alignment) const;
		vector<AlignmentPath> findPairedAlignmentPaths(const AlignmentType & alignment_1, const AlignmentType & alignment_2) const;

	private:

       	const PathsIndex & paths_index;
       	const string library_type;

       	const uint32_t max_pair_frag_length;

       	const uint32_t min_mapq_filter;
       	const double min_best_score_filter;
       	const double max_softclip_filter;

		bool alignmentHasPath(const vg::Alignment & alignment) const;
		bool alignmentHasPath(const vg::MultipathAlignment & alignment) const;
		
       	bool alignmentStartInGraph(const AlignmentType & alignment) const;

		vector<AlignmentSearchPath> extendAlignmentPath(const AlignmentSearchPath & align_search_path, const vg::Alignment & alignment) const;
		vector<AlignmentSearchPath> extendAlignmentPath(const AlignmentSearchPath & align_search_path, const vg::Alignment & alignment, const pair<uint32_t, uint32_t> start_node_path_idx) const;

		void extendAlignmentPath(vector<AlignmentSearchPath> * align_search_paths, const vg::Path & path) const;
		void extendAlignmentPath(AlignmentSearchPath * align_search_path, const vg::Mapping & mapping) const;

		vector<AlignmentSearchPath> extendAlignmentPath(const AlignmentSearchPath & align_search_path, const vg::MultipathAlignment & alignment) const;
		vector<AlignmentSearchPath> extendAlignmentPath(const AlignmentSearchPath & align_search_path, const vg::MultipathAlignment & alignment, const pair<uint32_t, uint32_t> start_node_path_idx) const;
		void extendAlignmentPaths(vector<AlignmentSearchPath> * align_search_paths, const google::protobuf::RepeatedPtrField<vg::Subpath> & subpaths, const pair<uint32_t, uint32_t> start_node_path_idx) const;

		void pairAlignmentPaths(vector<AlignmentSearchPath> * paired_align_search_paths, const AlignmentType & start_alignment, const AlignmentType & end_alignment) const;

		multimap<gbwt::node_type, pair<uint32_t, uint32_t> > getAlignmentStartNodesIndex(const vg::Alignment & alignment, const uint32_t max_internal_offset) const;
		multimap<gbwt::node_type, pair<uint32_t, uint32_t> > getAlignmentStartNodesIndex(const vg::MultipathAlignment & alignment, const uint32_t max_internal_offset) const;

		bool isAlignmentDisconnected(const vg::Alignment & alignment) const;
		bool isAlignmentDisconnected(const vg::MultipathAlignment & alignment) const;

		bool filterAlignmentSearchPaths(const vector<AlignmentSearchPath> & align_search_paths) const;
};


#endif
