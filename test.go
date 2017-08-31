package main

import (
	"database/sql"
	"fmt"
	"github.com/golang/protobuf/proto"
	_ "github.com/lib/pq"
	"github.com/sauercrowd/protobuf-go-c-test/pkg/test_proto"

	"log"
)

const DBHOST = "127.0.0.1"
const DBPORT = 5432
const DBNAME = "test_proto"
const DBUSER = "postgres"
const DBPASS = "postgres"
const QUERY = "SELECT data FROM test"

// first column id(serial), second column data(bytea)
const DBTABLE = "test"

type data struct {
	Bytes []byte
}

func main() {
	// setup database
	db, err := sql.Open("postgres", fmt.Sprintf("host=%s port=%d user=%s password=%s dbname=%s sslmode=disable", DBHOST, DBPORT, DBUSER, DBPASS, DBNAME))
	if err != nil {
		log.Fatal(err)
	}

	// query for data
	var d data
	err = db.QueryRow(QUERY).Scan(&d.Bytes)
	if err == sql.ErrNoRows {
		log.Fatal("No rows")
	}
	if err != nil {
		log.Fatal(err)
	}

	// try to deserialize
	test := &test_proto.TestMessage{}
	if err = proto.Unmarshal(d.Bytes, test); err != nil {
		log.Print("Failed to parse test Message", err)
		log.Printf("Got %d bytes", len(d.Bytes))
		log.Fatalf("content: %x", d.Bytes)
	}
	fmt.Printf("Message:\n\tinput: %f\n\toutput: %f\n\tinfo: %f\n", test.Input, test.Output, test.Info)
}
