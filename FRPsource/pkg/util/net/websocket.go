package net

import (
	"errors"
	"fmt"
	"net"
	"net/http"
	"net/url"
	"strings"
	"time"

	"golang.org/x/net/websocket"
)

var (
	ErrWebsocketListenerClosed = errors.New("websocket listener closed")
)

const (
	FrpWebsocketPath = "/~!auth"
)

type WebsocketListener struct {
	ln       net.Listener
	acceptCh chan net.Conn

	server    *http.Server
	httpMutex *http.ServeMux
}

// NewWebsocketListener to handle websocket connections
// ln: tcp listener for websocket connections
func NewWebsocketListener(ln net.Listener) (wl *WebsocketListener) {
	wl = &WebsocketListener{
		acceptCh: make(chan net.Conn),
	}

	muxer := http.NewServeMux()
	muxer.Handle(FrpWebsocketPath, websocket.Handler(func(c *websocket.Conn) {
		notifyCh := make(chan struct{})
		conn := WrapCloseNotifyConn(c, func() {
			close(notifyCh)
		})
		wl.acceptCh <- conn
		<-notifyCh
	}))

	wl.server = &http.Server{
		Addr:    ln.Addr().String(),
		Handler: muxer,
	}

	go wl.server.Serve(ln)
	return
}

func ListenWebsocket(bindAddr string, bindPort int) (*WebsocketListener, error) {
	tcpLn, err := net.Listen("tcp", fmt.Sprintf("%s:%d", bindAddr, bindPort))
	if err != nil {
		return nil, err
	}
	l := NewWebsocketListener(tcpLn)
	return l, nil
}

func (p *WebsocketListener) Accept() (net.Conn, error) {
	c, ok := <-p.acceptCh
	if !ok {
		return nil, ErrWebsocketListenerClosed
	}
	return c, nil
}

func (p *WebsocketListener) Close() error {
	return p.server.Close()
}

func (p *WebsocketListener) Addr() net.Addr {
	return p.ln.Addr()
}

// addr: domain:port
func ConnectWebsocketServer(addr string, websocket_domain string, isSecure bool) (net.Conn, error) {
	if isSecure {
		ho := strings.Split(addr, ":")
		ip, err := net.ResolveIPAddr("ip", ho[0])
		ip_addr := ip.String() + ":" + ho[1]
		if err != nil {
			return nil, err
		}
		addr = "wss://" + ip_addr + FrpWebsocketPath
	} else {
		addr = "ws://" + addr + FrpWebsocketPath
	}
	uri, err := url.Parse(addr)
	if err != nil {
		return nil, err
	}

	var origin string
	if isSecure {
		ho := strings.Split(uri.Host, ":")
		ip, err := net.ResolveIPAddr("ip", ho[0])
		ip_addr := ip.String() + ":" + ho[1]
		if err != nil {
			return nil, err
		}
		origin = "https://" + ip_addr
	} else {
		origin = "http://" + uri.Host
	}

	cfg, err := websocket.NewConfig(addr, origin)
	if err != nil {
		return nil, err
	}
	cfg.Dialer = &net.Dialer{
		Timeout: 10 * time.Second,
	}

	conn, err := websocket.DialConfig(cfg)
	if err != nil {
		return nil, err
	}
	return conn, nil
}
