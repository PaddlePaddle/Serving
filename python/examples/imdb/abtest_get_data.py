from paddle_serving_app.reader.imdb_reader import IMDBDataset
imdb_dataset = IMDBDataset()
imdb_dataset.load_resource('imdb.vocab')

with open('test_data/part-0') as fin:
    with open('processed.data', 'w') as fout:
        for line in fin:
            word_ids, label = imdb_dataset.get_words_and_label(line)
            fout.write("{};{}\n".format(','.join([str(x) for x in word_ids]), label[0]))
