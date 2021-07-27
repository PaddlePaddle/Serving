package main

import (
       "io"
       "os"
       "fmt"
       "bufio"
       "strings"
       "strconv"
)

func main() {
     score_file := os.Args[1]
     fi, err := os.Open(score_file)
     if err != nil {
     	fmt.Print(err)
     }

     defer fi.Close()
     br := bufio.NewReader(fi)     

     total := int(0)
     acc := int(0)
     for {
     	 line, err := br.ReadString('\n')
	 if err == io.EOF {
	    break
	 }

	 line = strings.Trim(line, "\n")
	 s := strings.Split(line, "\t")
	 prob_str := strings.Trim(s[0], " ")
	 label_str := strings.Trim(s[1], " ")
	 prob, err := strconv.ParseFloat(prob_str, 32)
	 if err != nil {
	    panic(err)
	 }
	 label, err := strconv.ParseFloat(label_str, 32)
	 if err != nil {
	    panic(err)
	 }
	 if (prob - 0.5) * (label - 0.5) > 0 {
	    acc++
	 }
	 total++
    }
    fmt.Println("total num: ", total)
    fmt.Println("acc num: ", acc)
    fmt.Println("acc: ", float32(acc) / float32(total))    
}