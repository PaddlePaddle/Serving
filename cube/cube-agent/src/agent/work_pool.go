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
