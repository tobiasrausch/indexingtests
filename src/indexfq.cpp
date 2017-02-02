#include <sdsl/suffix_arrays.hpp>
#include <htslib/faidx.h>
#include <fstream>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem.hpp>
#include <boost/progress.hpp>


struct Config {
  std::string outprefix;
  std::vector<boost::filesystem::path> fastq;
};

using namespace sdsl;

int main(int argc, char** argv) {
  Config c;
  // Parameter
  boost::program_options::options_description generic("Generic options");
  generic.add_options()
    ("help,?", "show help message")
    ("outprefix,o", boost::program_options::value<std::string>(&c.outprefix)->default_value("fmidx"), "output file prefix")
    ;

  boost::program_options::options_description hidden("Hidden options");
  hidden.add_options()
    ("input-file", boost::program_options::value< std::vector<boost::filesystem::path> >(&c.fastq), "input fastq files")
    ;

  boost::program_options::positional_options_description pos_args;
  pos_args.add("input-file", -1);

  boost::program_options::options_description cmdline_options;
  cmdline_options.add(generic).add(hidden);
  boost::program_options::options_description visible_options;
  visible_options.add(generic);
  boost::program_options::variables_map vm;
  boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(pos_args).run(), vm);
  boost::program_options::notify(vm);

  // Check command line arguments
  if ((vm.count("help")) || (!vm.count("input-file"))) {
    std::cout << "Usage: " << argv[0] << " [OPTIONS] -o <output.prefix> <read1.fq.gz> <read2.fq.gz> ..." << std::endl;
    std::cout << visible_options << "\n";
    return 1;
  } 

  boost::filesystem::path outfile(c.outprefix + ".dump");
  std::string index_file = c.outprefix + ".fm9";
  csa_wt<wt_huff<bit_vector,rank_support_v5<>,select_support_scan<>,select_support_scan<0> >,1<<20,1<<20> fm_index;

  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
  std::cout << '[' << boost::posix_time::to_simple_string(now) << "] " << "Dumping FASTQ" << std::endl;
  boost::progress_display show_progress( c.fastq.size() );

  // Dump fastq
  std::ofstream tmpout(outfile.string().c_str());
  for(uint32_t i = 0; i < c.fastq.size(); ++i) {
    std::ifstream file(c.fastq[i].string().c_str(), std::ios_base::in | std::ios_base::binary);
    boost::iostreams::filtering_streambuf<boost::iostreams::input> dataIn;
    dataIn.push(boost::iostreams::gzip_decompressor());
    dataIn.push(file);
    std::istream instream(&dataIn);
    std::string line;
    int32_t lcount = 0;
    while(std::getline(instream, line))
      if (lcount++ % 4 == 1) tmpout << line << std::endl;
    file.close();
    ++show_progress;
  }
  tmpout.close();

  now = boost::posix_time::second_clock::local_time();
  std::cout << '[' << boost::posix_time::to_simple_string(now) << "] " << "Building FM-Index" << std::endl;

  // Build index
  construct(fm_index, outfile.string().c_str(), 1);
  store_to_file(fm_index, index_file);
  boost::filesystem::remove(outfile);

  now = boost::posix_time::second_clock::local_time();
  std::cout << '[' << boost::posix_time::to_simple_string(now) << "] " << "Done." << std::endl;
}
