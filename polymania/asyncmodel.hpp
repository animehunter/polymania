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

        AsyncState(AsyncCompletionHolderBase *completionRoutine) : isComplete(false), completionRoutine(completionRoutine), onComplete(0) {}
    };

public:
    // construct a synchronous result
    AsyncResult() {
    }

    // construct an asynchronous result
    template<typename F>
    AsyncResult(F completionFunc) {
        asyncState = std::shared_ptr<AsyncState>(new AsyncState(new AsyncCompletionHolder<F>(completionFunc)));
    }

    ~AsyncResult() {
        if(asyncState) {
            if(asyncState->onComplete) delete asyncState->onComplete;
            if(asyncState->completionRoutine) delete asyncState->completionRoutine;
        }
    }

    template<typename F>
    inline AsyncResult<T> &SetCompletionCallback(F inOnComplete) {
        if(asyncState) {
            if(asyncState->onComplete) delete asyncState->onComplete;
            asyncState->onComplete = new AsyncCompletionHolder<F>(onComplete);
        }
        return *this;
    }

    inline void ClearCompletionCallback() {
        if(asyncState) {
            if(asyncState->onComplete) delete asyncState->onComplete;
            asyncState->onComplete  0;
        }
    }

    inline bool IsComplete() const { 
        return asyncState ? asyncState->isComplete : true;
    }
    inline void GetResult(T &outResult) const { 
        if(asyncState) {
            asyncState->completionRoutine->Call();
            outResult = asyncState->result;
        } else {
            outResult = syncResult;
        }
    }

public:
    T syncResult;
    std::shared_ptr<AsyncState> asyncState;
};


