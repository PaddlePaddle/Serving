from plugin_service import PluginService
import sys

class IMDBService(PluginService):
    def prepare_service(self, args={}):
        if len(args) == 0:
            exit(-1)
        self.word_dict = {}
        with open(args["dict_file_path"]) as fin:
            idx = 0
            for line in fin:
                self.word_dict[idx] = idx
                idx += 1

    def preprocess(self, feed={}, fetch=[]):
        if "words" not in feed:
            exit(-1)
        res_feed = {}
        #res_feed["words"] = [self.word_dict[int(x)] for x in feed["words"]]
        res_feed['words'] = feed['words']
        return res_feed, fetch

imdb_service = IMDBService(name="imdb", model=sys.argv[1], port=9294)
imdb_service.start_service()
