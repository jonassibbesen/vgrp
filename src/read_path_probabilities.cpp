
#include "read_path_probabilities.hpp"

#include <assert.h>
#include <algorithm>
#include <numeric>
#include <limits>


ReadPathProbabilities::ReadPathProbabilities() : score_log_base(1) {

    noise_prob = 1;     
}

ReadPathProbabilities::ReadPathProbabilities(const uint32_t num_paths, const double score_log_base_in) : score_log_base(score_log_base_in) {

    noise_prob = 1;
    read_path_probs = vector<double>(num_paths, 0);
}

void ReadPathProbabilities::calcReadPathProbabilities(const vector<AlignmentPath> & align_paths, const unordered_map<uint32_t, uint32_t> & clustered_path_index, const FragmentLengthDist & fragment_length_dist, const bool is_single_end) {

    assert(!align_paths.empty());
    assert(clustered_path_index.size() == read_path_probs.size());

    if (align_paths.front().mapq_comb > 0) {

        noise_prob = phred_to_prob(align_paths.front().mapq_comb);
        assert(noise_prob < 1);

        vector<double> align_paths_log_probs;
        align_paths_log_probs.reserve(align_paths.size());

        double align_paths_log_probs_sum = numeric_limits<double>::lowest();

        for (auto & align_path: align_paths) {

            align_paths_log_probs.emplace_back(score_log_base * align_path.score_sum);

            if (!is_single_end) {

                align_paths_log_probs.back() += fragment_length_dist.logProb(align_path.seq_length);
            }

            align_paths_log_probs_sum = add_log(align_paths_log_probs_sum, align_paths_log_probs.back());
        }

        for (auto & log_probs: align_paths_log_probs) {

            log_probs -= align_paths_log_probs_sum;
        }

        double read_path_probs_sum = 0;

        for (size_t i = 0; i < align_paths.size(); ++i) {

            for (auto & path: align_paths.at(i).ids) {

                read_path_probs.at(clustered_path_index.at(path)) = exp(align_paths_log_probs.at(i));
            }

            read_path_probs_sum += exp(align_paths_log_probs.at(i)) * align_paths.at(i).ids.size();
        }

        assert(read_path_probs_sum > 0);

        for (auto & probs: read_path_probs) {

            probs /= read_path_probs_sum;
            probs *= (1 - noise_prob);
        }
    }
}

void ReadPathProbabilities::addPositionalProbabilities(const vector<double> & path_lengths) {

    assert(path_lengths.size() == read_path_probs.size());

    if (noise_prob < 1) {

        double read_path_probs_sum = 0;

        for (size_t i = 0; i < read_path_probs.size(); ++i) {

            if (doubleCompare(path_lengths.at(i), 0)) {

                read_path_probs.at(i) = 0;

            } else {

                read_path_probs.at(i) /= path_lengths.at(i);
            }

            read_path_probs_sum += read_path_probs.at(i);
        }

        assert(read_path_probs_sum > 0);

        for (auto & probs: read_path_probs) {

            probs /= read_path_probs_sum;
            probs *= (1 - noise_prob);
        }  
    }
}

double ReadPathProbabilities::calcReadMappingProbabilities(const vg::Alignment & alignment, const vector<double> & quality_match_probs, const vector<double> & quality_mismatch_probs, const double indel_prob) const {

    double align_path_prob = 0;

    auto & base_qualities = alignment.quality();
    uint32_t cur_pos = 0;

    for (auto & mapping: alignment.path().mapping()) {

        for (auto & edit: mapping.edit()) {

            if (edit.from_length() == edit.to_length() && edit.sequence().empty()) {

                for (uint32_t i = cur_pos; i < cur_pos + edit.from_length(); ++i) {

                    align_path_prob += quality_match_probs.at(uint32_t(base_qualities.at(i)));
                }

            } else if (edit.from_length() == edit.to_length() && !edit.sequence().empty()) {

                for (uint32_t i = cur_pos; i < cur_pos + edit.from_length(); ++i) {

                    align_path_prob += quality_mismatch_probs.at(uint32_t(base_qualities.at(i)));
                }
            
            } else if (edit.from_length() == 0 && edit.to_length() > 0 && !edit.sequence().empty()) {

                align_path_prob += edit.to_length() * indel_prob;

            } else if (edit.from_length() > 0 && edit.to_length() == 0) {

                align_path_prob += edit.from_length() * indel_prob;
            }
        }
    } 

    return align_path_prob;
}

bool operator==(const ReadPathProbabilities & lhs, const ReadPathProbabilities & rhs) { 

    if (abs(lhs.noise_prob - rhs.noise_prob) < probability_precision) {

        if (lhs.read_path_probs.size() == rhs.read_path_probs.size()) {

            for (size_t i = 0; i < lhs.read_path_probs.size(); ++i) {

                if (abs(lhs.read_path_probs.at(i) - rhs.read_path_probs.at(i)) >= probability_precision) {

                    return false;
                }
            }

            return true;
        }
    } 

    return false;
}

bool operator!=(const ReadPathProbabilities & lhs, const ReadPathProbabilities & rhs) { 

    return !(lhs == rhs);
}

bool operator<(const ReadPathProbabilities & lhs, const ReadPathProbabilities & rhs) { 

    if (lhs.noise_prob != rhs.noise_prob) {

        return (lhs.noise_prob < rhs.noise_prob);    
    } 

    assert(lhs.read_path_probs.size() == rhs.read_path_probs.size());

    for (size_t i = 0; i < lhs.read_path_probs.size(); ++i) {

        if (lhs.read_path_probs.at(i) != rhs.read_path_probs.at(i)) {

            return (lhs.read_path_probs.at(i) < rhs.read_path_probs.at(i));    
        }         
    }   

    return false;
}

ostream & operator<<(ostream & os, const ReadPathProbabilities & probs) {

    os << probs.noise_prob;

    for (auto & prob: probs.read_path_probs) {

        os << " " << prob;
    }

    return os;
}
