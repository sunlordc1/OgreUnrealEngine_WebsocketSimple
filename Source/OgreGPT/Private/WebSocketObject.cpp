#include "WebSocketObject.h"
#include "WebSocketsModule.h"
#include "Modules/ModuleManager.h"
#include "Async/Async.h"
#include "Misc/CoreDelegates.h"
#include "Engine/Engine.h"

UWebSocketObject* UWebSocketObject::Connect(const FString& ServerUrl)
{
    if (!FModuleManager::Get().IsModuleLoaded(TEXT("WebSockets")))
    {
        FModuleManager::Get().LoadModuleChecked<FWebSocketsModule>(TEXT("WebSockets"));
    }

    UWebSocketObject* Obj = NewObject<UWebSocketObject>();
    if (!Obj)
    {
        UE_LOG(LogTemp, Warning, TEXT("UWebSocketObject::Connect - NewObject failed"));
        return nullptr;
    }

    Obj->AddToRoot();
    Obj->Init(ServerUrl);
    return Obj;
}

void UWebSocketObject::Init(const FString& ServerUrl)
{
    if (WebSocket.IsValid())
    {
        WebSocket->OnConnected().RemoveAll(this);
        WebSocket->OnMessage().RemoveAll(this);
        WebSocket->OnClosed().RemoveAll(this);
        WebSocket->OnConnectionError().RemoveAll(this);

        if (WebSocket->IsConnected())
        {
            WebSocket->Close();
        }
        WebSocket.Reset();
    }

    WebSocket = FWebSocketsModule::Get().CreateWebSocket(ServerUrl);

    if (!WebSocket.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UWebSocketObject::Init - CreateWebSocket returned null for URL: %s"), *ServerUrl);
        AsyncTask(ENamedThreads::GameThread, [this]()
        {
            OnConnectionError.Broadcast(TEXT("CreateWebSocket returned null"));
        });
        return;
    }

    WebSocket->OnConnected().AddUObject(this, &UWebSocketObject::HandleConnected);
    WebSocket->OnMessage().AddUObject(this, &UWebSocketObject::HandleMessage);
    WebSocket->OnClosed().AddUObject(this, &UWebSocketObject::HandleClosed);
    WebSocket->OnConnectionError().AddUObject(this, &UWebSocketObject::HandleConnectionError);

    OnPreExitHandle = FCoreDelegates::OnPreExit.AddUObject(this, &UWebSocketObject::HandleAppPreExit);
    OnWorldTearDownHandle = FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &UWebSocketObject::HandleWorldTearDown);

    WebSocket->Connect();
}

void UWebSocketObject::SendMessage(const FString& Message)
{
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        WebSocket->Send(Message);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UWebSocketObject::SendMessage - websocket is not connected"));
    }
}

void UWebSocketObject::Close()
{
    if (WebSocket.IsValid())
    {
        WebSocket->OnConnected().RemoveAll(this);
        WebSocket->OnMessage().RemoveAll(this);
        WebSocket->OnClosed().RemoveAll(this);
        WebSocket->OnConnectionError().RemoveAll(this);

        if (WebSocket->IsConnected())
        {
            WebSocket->Close();
        }

        WebSocket.Reset();
    }

    if (OnPreExitHandle.IsValid())
    {
        FCoreDelegates::OnPreExit.Remove(OnPreExitHandle);
        OnPreExitHandle.Reset();
    }

    if (OnWorldTearDownHandle.IsValid())
    {
        FWorldDelegates::OnWorldBeginTearDown.Remove(OnWorldTearDownHandle);
        OnWorldTearDownHandle.Reset();
    }
}

void UWebSocketObject::Disconnect()
{
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        WebSocket->Close();
        AsyncTask(ENamedThreads::GameThread, [this]()
        {
            OnClosed.Broadcast(1000, TEXT("Manual Disconnect"), true);
        });
    }
    Close();
}

void UWebSocketObject::HandleConnected()
{
    AsyncTask(ENamedThreads::GameThread, [this]()
    {
        OnConnected.Broadcast();
    });
}

void UWebSocketObject::HandleMessage(const FString& Message)
{
    const FString Local = Message;
    AsyncTask(ENamedThreads::GameThread, [this, Local]()
    {
        OnMessage.Broadcast(Local);
    });
}

void UWebSocketObject::HandleClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    const int32 LocalCode = StatusCode;
    const FString LocalReason = Reason;
    const bool LocalClean = bWasClean;
    AsyncTask(ENamedThreads::GameThread, [this, LocalCode, LocalReason, LocalClean]()
    {
        OnClosed.Broadcast(LocalCode, LocalReason, LocalClean);
    });
}

void UWebSocketObject::HandleConnectionError(const FString& Error)
{
    const FString LocalError = Error;
    AsyncTask(ENamedThreads::GameThread, [this, LocalError]()
    {
        OnConnectionError.Broadcast(LocalError);
    });
}

void UWebSocketObject::HandleAppPreExit()
{
    Disconnect();
}

void UWebSocketObject::HandleWorldTearDown(UWorld* World)
{
    Disconnect();
}

void UWebSocketObject::BeginDestroy()
{
    Close();

    if (IsRooted())
    {
        RemoveFromRoot();
    }

    Super::BeginDestroy();
}
