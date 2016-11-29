{
    "targets": [
        { "target_name": "word_vec",
          "sources": ["word_vec.cpp",
                      "cpp/compressor/compressor.cpp",
                      "cpp/parser/parser.cpp",
                      "cpp/semantics/semantics.cpp",
                      "cpp/tagger/crf.cpp",
                      "cpp/tagger/crfpos.cpp",
                      "cpp/tagger/la_pos.cpp",
                      "cpp/tagger/lookahead.cpp",
                      "cpp/tagger/tokenize.cpp" ],
         "include_dirs": ["/usr/local/include", 
                          "/usr/include"],
         "libraries": ["-lboost_serialization", "-L/usr/local/lib",
                       "-lwordnet", "-L/usr/lib",
                       "-lsmnet", "-L/usr/local/lib"],
         "cflags_cc": ["-std=c++11", "-fexceptions"],
         "cflags_cc!": ["-fno-rtti"],
        }
    ]
}
