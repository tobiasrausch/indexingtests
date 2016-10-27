#include <sdsl/suffix_arrays.hpp>
#include <htslib/faidx.h>
#include <fstream>


using namespace sdsl;

int main(int argc, char** argv) {

  std::string outfile("bla.gmask");
  std::string index_file = outfile + ".fm9";
  csa_wt<wt_huff<bit_vector,rank_support_v5<>,select_support_scan<>,select_support_scan<0> >,1<<20,1<<20> fm_index;

  
  /*
  // Load the genome
  std::ofstream tmpout(outfile.c_str());
  faidx_t* fai = fai_load(argv[1]);
  for(int32_t refIndex = 0; refIndex < faidx_nseq(fai); ++refIndex) {
    std::cout << refIndex << std::endl;
    std::string seqname(faidx_iseq(fai, refIndex));
    int32_t seqlen = -1;
    char* seq = faidx_fetch_seq(fai, seqname.c_str(), 0, faidx_seq_len(fai, seqname.c_str()), &seqlen);
    tmpout << seq << std::endl;
    if (seqlen) free(seq);
  }
  fai_destroy(fai);
  tmpout.close();

  // Build index
  std::cout << "Index genome" << std::endl;
  construct(fm_index, outfile, 1);
  store_to_file(fm_index, index_file);
  */
  
  // Build mappability map
  int32_t isize = 300;
  int32_t k = 55;
  int32_t hbin = isize / 2;
  if (!load_from_file(fm_index, index_file)) {
    std::cerr << "Genome index could not be loaded!" << std::endl;
    exit(-1);
  }
  std::ofstream ofile(outfile.c_str());
  faidx_t* fai = fai_load(argv[1]);
  for(int32_t refIndex = 0; refIndex < faidx_nseq(fai); ++refIndex) {
    std::cout << refIndex << std::endl;
    std::string seqname(faidx_iseq(fai, refIndex));
    ofile << ">" << seqname << std::endl;
    int32_t seqlen = -1;
    char* seq = faidx_fetch_seq(fai, seqname.c_str(), 0, faidx_seq_len(fai, seqname.c_str()), &seqlen);
    for (int32_t i = 0; ((i < hbin) && (i < seqlen)); ++i) ofile << 'N';	
    if (hbin < seqlen) {
      std::string lquery = std::string(seq, seq + k);
      std::string rquery = std::string(seq + 2 * hbin - k, seq + 2 * hbin);
      for (int32_t i = hbin; i < seqlen - hbin; ++i) {
	std::size_t loccs = sdsl::count(fm_index, lquery.begin(), lquery.end());
	std::size_t roccs = sdsl::count(fm_index, rquery.begin(), rquery.end());
	if ((loccs > 1) || (roccs > 1)) ofile << 'N';
	else ofile << 'A';
	lquery.erase(0, 1);
	rquery.erase(0, 1);
	lquery += seq[(i - hbin + k) + 1];
	rquery += seq[(i + hbin) + 1];
      }
    }
    for (int32_t i = std::max(hbin, seqlen - hbin); i < seqlen; ++i) ofile << 'N';
    ofile << std::endl;
    if (seqlen) free(seq);
  }
  fai_destroy(fai);
  ofile.close();
}
