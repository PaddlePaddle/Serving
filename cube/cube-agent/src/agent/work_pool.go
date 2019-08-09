// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package agent

import (
	"errors"
	"fmt"
	"sync"
	"sync/atomic"
)

type (
	workType struct {
		poolWorker    PoolWorker
		resultChannel chan error
	}

	WorkPool struct {
		queueChannel    chan workType
		workChannel     chan PoolWorker
		queuedWorkNum   int32
		activeWorkerNum int32
		queueCapacity   int32
		workFilter      sync.Map
	}
)

type PoolWorker interface {
	Token() string
	DoWork()
}

func NewWorkPool(workerNum int, queueCapacity int32) *WorkPool {
	workPool := WorkPool{
		queueChannel:    make(chan workType),
		workChannel:     make(chan PoolWorker, queueCapacity),
		queuedWorkNum:   0,
		activeWorkerNum: 0,
		queueCapacity:   queueCapacity,
	}

	for i := 0; i < workerNum; i++ {
		go workPool.startWorkRoutine()
	}

	go workPool.startQueueRoutine()

	return &workPool
}

func (workPool *WorkPool) startWorkRoutine() {
	for {
		select {
		case work := <-workPool.workChannel:
			workPool.doWork(work)
			break
		}
	}
}

func (workPool *WorkPool) startQueueRoutine() {
	for {
		select {
		case queueItem := <-workPool.queueChannel:
			if atomic.AddInt32(&workPool.queuedWorkNum, 0) == workPool.queueCapacity {
				queueItem.resultChannel <- fmt.Errorf("work pool fulled with %v pending works", QueueCapacity)
				continue
			}

			atomic.AddInt32(&workPool.queuedWorkNum, 1)

			workPool.workChannel <- queueItem.poolWorker

			queueItem.resultChannel <- nil

			break
		}
	}
}

func (workPool *WorkPool) doWork(poolWorker PoolWorker) {
	defer atomic.AddInt32(&workPool.activeWorkerNum, -1)
	defer workPool.workFilter.Delete(poolWorker.Token())

	atomic.AddInt32(&workPool.queuedWorkNum, -1)
	atomic.AddInt32(&workPool.activeWorkerNum, 1)

	poolWorker.DoWork()
}

func (workPool *WorkPool) PostWorkWithToken(poolWorker PoolWorker) (err error) {
	if _, ok := workPool.workFilter.Load(poolWorker.Token()); ok {
		return errors.New("another work with same key is doing.")
	}
	workPool.workFilter.Store(poolWorker.Token(), true)
	return workPool.PostWork(poolWorker)
}

func (workPool *WorkPool) PostWork(poolWorker PoolWorker) (err error) {
	work := workType{poolWorker, make(chan error)}

	defer close(work.resultChannel)

	workPool.queueChannel <- work

	err = <-work.resultChannel

	return err
}
