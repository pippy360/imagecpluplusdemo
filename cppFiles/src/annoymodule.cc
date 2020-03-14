// Copyright (c) 2013 Spotify AB
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

#include "annoylib.h"
#include "kissrandom.h"

#include <exception>
#if defined(_MSC_VER) && _MSC_VER == 1500
typedef signed __int32    int32_t;
#else
#include <stdint.h>
#endif

template class AnnoyIndexInterface<int32_t, float>;

class HammingWrapper : public AnnoyIndexInterface<int32_t, float> {
  // Wrapper class for Hamming distance, using composition.
  // This translates binary (float) vectors into packed uint64_t vectors.
  // This is questionable from a performance point of view. Should reconsider this solution.

public:

	int32_t _f_external, _f_internal;
  AnnoyIndex<int32_t, uint64_t, Hamming, Kiss64Random> _index;
  void _pack(const float* src, uint64_t* dst) const {
	for (int32_t i = 0; i < _f_internal; i++) {
	  dst[i] = 0;
	  for (int32_t j = 0; j < 64 && i*64+j < _f_external; j++) {
	dst[i] |= (uint64_t)(src[i * 64 + j] > 0.5) << j;
	  }
	}
  };
  void _unpack(const uint64_t* src, float* dst) const {
	for (int32_t i = 0; i < _f_external; i++) {
	  dst[i] = (src[i / 64] >> (i % 64)) & 1;
	}
  };
  HammingWrapper(int f) : _f_external(f), _f_internal((f + 63) / 64), _index((f + 63) / 64) {};
  bool add_item(int32_t item, const float* w, char**error) {
	vector<uint64_t> w_internal(_f_internal, 0);
	_pack(w, &w_internal[0]);
	return _index.add_item(item, &w_internal[0], error);
  };
  bool build(int q, char** error) { return _index.build(q, error); };
  bool unbuild(char** error) { return _index.unbuild(error); };
  bool save(const char* filename, bool prefault, char** error) { return _index.save(filename, prefault, error); };
  void unload() { _index.unload(); };
  bool load(const char* filename, bool prefault, char** error) { return _index.load(filename, prefault, error); };
  float get_distance(int32_t i, int32_t j) const { return _index.get_distance(i, j); };
  void get_nns_by_item(int32_t item, size_t n, int search_k, vector<int32_t>* result, vector<float>* distances) const {
	if (distances) {
	  vector<uint64_t> distances_internal;
	  _index.get_nns_by_item(item, n, search_k, result, &distances_internal);
	  distances->insert(distances->begin(), distances_internal.begin(), distances_internal.end());
	} else {
	  _index.get_nns_by_item(item, n, search_k, result, NULL);
	}
  };
  void get_nns_by_vector(const float* w, size_t n, int search_k, vector<int32_t>* result, vector<float>* distances) const {
	vector<uint64_t> w_internal(_f_internal, 0);
	_pack(w, &w_internal[0]);
	if (distances) {
	  vector<uint64_t> distances_internal;
	  _index.get_nns_by_vector(&w_internal[0], n, search_k, result, &distances_internal);
	  distances->insert(distances->begin(), distances_internal.begin(), distances_internal.end());
	} else {
	  _index.get_nns_by_vector(&w_internal[0], n, search_k, result, NULL);
	}
  };
  int32_t get_n_items() const { return _index.get_n_items(); };
  int32_t get_n_trees() const { return _index.get_n_trees(); };
  void verbose(bool v) { _index.verbose(v); };
  void get_item(int32_t item, float* v) const {
	vector<uint64_t> v_internal(_f_internal, 0);
	_index.get_item(item, &v_internal[0]);
	_unpack(&v_internal[0], v);
  };
  void set_seed(int q) { _index.set_seed(q); };
  bool on_disk_build(const char* filename, char** error) { return _index.on_disk_build(filename, error); };
};

