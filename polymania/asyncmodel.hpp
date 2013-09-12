#pragma once

template<typename T>
struct AsyncResult {
private:
    struct AsyncCompletionHolderBase {
        AsyncCompletionHolderBase(){}
        virtual ~AsyncCompletionHolderBase(){}
        virtual void Call()=0;
    };
    template<typename F>
    struct AsyncCompletionHolder : public AsyncCompletionHolderBase {
        F onComplete;
        void Call() {
            F();
        }
    };

    struct AsyncState{
        T result;
        bool isComplete;
        AsyncCompletionHolderBase *completionRoutine;
        AsyncCompletionHolderBase *onComplete;

        AsyncState(AsyncCompletionHolderBase *completionRoutine, AsyncCompletionHolderBase *onComplete) : isComplete(false), completionRoutine(completionRoutine), onComplete(onComplete) {}
    };

public:
    // construct a synchronous result
    AsyncResult() {
    }

    // construct an asynchronous result
    template<typename FCompletion>
    AsyncResult(FCompletion completionFunc) {
        auto completion = new AsyncCompletionHolder<FCompletion>(completionFunc);
        asyncState = std::shared_ptr<AsyncState>(new AsyncState(completion, 0));
    }

    // construct an asynchronous result
    template<typename FCompletion, typename FOnComplete>
    AsyncResult(FCompletion completionFunc, FOnComplete onComplete) {
        auto completion = new AsyncCompletionHolder<FCompletion>(completionFunc);
        auto complete = new AsyncCompletionHolder<FOnComplete>(onComplete);
        asyncState = std::shared_ptr<AsyncState>(new AsyncState(completion, complete));
    }

    ~AsyncResult() {
        if(asyncState) {
            if(asyncState->onComplete) delete asyncState->onComplete;
            if(asyncState->completionRoutine) delete asyncState->completionRoutine;
        }
    }

    // poll completion status (nonblocking)
    inline bool IsComplete() const { 
        return asyncState ? asyncState->isComplete : true;
    }

    // Get the result, will block until the IO request is complete
    inline T GetResult() const { 
        if(asyncState) {
            asyncState->completionRoutine->Call();
            return asyncState->result;
        } else {
            return syncResult;
        }
    }

public:
    T syncResult;
    std::shared_ptr<AsyncState> asyncState;
};


