#!/usr/bin/node
var wv = require('./build/Release/word_vec.node');
var fs = require('fs');
// Set Args
var argv = require('yargs')
    .usage('Usage: $0 --dataset [string] --filter [num] --x [num] --y [num]')
    .alias('f', 'filter')
    .alias('d', 'dataset')
    .demand(['d', 'f', 'x', 'y'])
    .describe('dataset', 'load a json dataset of reviews')
    .describe('filter', 'ignore reviews with more words than the filter value')
    .describe('x', 'encodable words thershold X')
    .describe('y', 'non-encodable words thershold Y')
    .argv;

console.log('dataset: '+argv.d+' filter: '+argv.f+
            ' X threshold: '+argv.x+' Y threshold: '+argv.y);

// parse the json dataset
var json = JSON.parse(require('fs').readFileSync(argv.d, 'utf8'));
if (json)
{
    var res = wv.compress_sparse(json, argv.f, argv.x, argv.y);
    if (res)
    {
        var output = 'sparse_vector_f'+argv.f+'_x'+argv.x+'_y'+argv.y+'.json';
        fs.writeFile(output, JSON.stringify(res), function(err) {
            if(err) {return console.log(err);}
        });
    }
    else
        console.log('failed to create sparse vector');
}
else
    console.log('failed to load dataset '+argv.d);
