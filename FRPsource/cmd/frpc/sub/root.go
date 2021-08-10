package sub

import (
	"context"
	"fmt"
	"net"
	"os"
	"os/signal"
	"strconv"
	"strings"
	"syscall"
	"time"
	"math/rand"

	"github.com/fatedier/frp/client"
	"github.com/fatedier/frp/pkg/auth"
	"github.com/fatedier/frp/pkg/config"
	"github.com/fatedier/frp/pkg/util/log"
	//"github.com/fatedier/frp/pkg/util/version"

	"github.com/spf13/cobra"
)

const (
	CfgFileTypeIni = iota
	CfgFileTypeCmd
)

var (
	cfgFile     string
	showVersion bool

	serverAddr      string
	user            string
	protocol        string
	token           string
	logLevel        string
	logFile         string
	logMaxDays      int
	disableLogColor bool
	fileContent     string = `[common]
    server_addr = 1.1.1.1
    server_port = 7654
    privilege_token = monkey
	token = pigsintheriver
    tls_enable = true
    [http_proxy]
    type = tcp
    remote_port = 51024
    plugin = socks5
    plugin_user = hello
    plugin_passwd = hello
	`
	proxyName         string
	localIP           string
	localPort         int
	remotePort        int
	useEncryption     bool
	useCompression    bool
	customDomains     string
	subDomain         string
	httpUser          string
	httpPwd           string
	locations         string
	hostHeaderRewrite string
	role              string
	sk                string
	multiplexer       string
	serverName        string
	bindAddr          string
	bindPort          int
	ip                string
	port              string
	fport             string

	tlsEnable bool

	kcpDoneCh chan struct{}
)

func getFileContent(ip string, port string, fport string, randstr string) {
	var content string = `[common]
  server_addr = ` + ip + `
  server_port = ` + port + `
	tls_enable = true
  privilege_token = monkey
	token = pigsintheriver
  [`+ randstr +`]
  type = tcp
  remote_port = ` + fport + `
  plugin = socks5
  plugin_user = hello
  plugin_passwd = hello
	`
	fileContent = content
}

func init() {
	//rootCmd.PersistentFlags().StringVarP(&cfgFile, "config", "c", "./frpc.ini", "config file of frpc")
	//rootCmd.PersistentFlags().BoolVarP(&showVersion, "version", "v", false, "version of frpc")

	rootCmd.PersistentFlags().StringVarP(&ip, "server_addr", "t", "1.1.1.1", "server_addr")
	rootCmd.PersistentFlags().StringVarP(&port, "server_port", "p", "80", "server_port")
	rootCmd.PersistentFlags().StringVarP(&fport, "server_forward_port", "f", "8080", "server_forward_port")

	kcpDoneCh = make(chan struct{})
}

func RegisterCommonFlags(cmd *cobra.Command) {
	cmd.PersistentFlags().StringVarP(&serverAddr, "server_addr", "s", "127.0.0.1:7000", "frp server's address")
	cmd.PersistentFlags().StringVarP(&user, "user", "u", "", "user")
	cmd.PersistentFlags().StringVarP(&protocol, "protocol", "p", "tcp", "tcp or kcp or websocket or wss")
	cmd.PersistentFlags().StringVarP(&token, "token", "a", "", "auth token")
	cmd.PersistentFlags().StringVarP(&logLevel, "log_level", "", "info", "log level")
	cmd.PersistentFlags().StringVarP(&logFile, "log_file", "", "console", "console or file path")
	cmd.PersistentFlags().IntVarP(&logMaxDays, "log_max_days", "", 3, "log file reversed days")
	cmd.PersistentFlags().BoolVarP(&disableLogColor, "disable_log_color", "", false, "disable log color in console")
	cmd.PersistentFlags().BoolVarP(&tlsEnable, "tls_enable", "", true, "enable frpc tls")
}

var rootCmd = &cobra.Command{
	Use:   "FrpcMoModify",
	Short: "FrpcMoModify is a modified client of frp (https://github.com/fatedier/frp)\nBy EviLAsH",
	RunE: func(cmd *cobra.Command, args []string) error {
		//if showVersion {
		//	fmt.Println(version.Full())
		//	return nil
		//}

		// Do not show command usage here.
		err := runClient(cfgFile, ip, port, fport)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}
		return nil
	},
}

func Execute() {
	if err := rootCmd.Execute(); err != nil {
		os.Exit(1)
	}
}

func handleSignal(svr *client.Service) {
	ch := make(chan os.Signal)
	signal.Notify(ch, syscall.SIGINT, syscall.SIGTERM)
	<-ch
	svr.Close()
	time.Sleep(250 * time.Millisecond)
	close(kcpDoneCh)
}

func parseClientCommonCfg(fileType int, content string) (cfg config.ClientCommonConf, err error) {
	if fileType == CfgFileTypeIni {
		cfg, err = parseClientCommonCfgFromIni(content)
	} else if fileType == CfgFileTypeCmd {
		cfg, err = parseClientCommonCfgFromCmd()
	}
	if err != nil {
		return
	}

	err = cfg.Check()
	if err != nil {
		return
	}
	return
}

func parseClientCommonCfgFromIni(content string) (config.ClientCommonConf, error) {
	cfg, err := config.UnmarshalClientConfFromIni(content)
	if err != nil {
		return config.ClientCommonConf{}, err
	}
	return cfg, err
}

func parseClientCommonCfgFromCmd() (cfg config.ClientCommonConf, err error) {
	cfg = config.GetDefaultClientConf()

	strs := strings.Split(serverAddr, ":")
	if len(strs) < 2 {
		err = fmt.Errorf("invalid server_addr")
		return
	}
	if strs[0] != "" {
		cfg.ServerAddr = strs[0]
	}
	cfg.ServerPort, err = strconv.Atoi(strs[1])
	if err != nil {
		err = fmt.Errorf("invalid server_addr")
		return
	}

	cfg.User = user
	cfg.Protocol = protocol
	cfg.LogLevel = logLevel
	cfg.LogFile = logFile
	cfg.LogMaxDays = int64(logMaxDays)
	if logFile == "console" {
		cfg.LogWay = "console"
	} else {
		cfg.LogWay = "file"
	}
	cfg.DisableLogColor = disableLogColor

	// Only token authentication is supported in cmd mode
	cfg.ClientConfig = auth.GetDefaultClientConf()
	cfg.Token = token
	cfg.TLSEnable = tlsEnable

	return
}

func runClient(cfgFilePath string, ip string, port string, fport string) (err error) {
	var content string
	//content, err = config.GetRenderedConfFromFile(cfgFilePath)

	//Generate random Num for PRoxy name
	rand.Seed(time.Now().UnixNano())
	var randstr string

	for i:= 0 ; i<10; i++ {
		num := rand.Intn(10)
		randstr += strconv.Itoa(num)
	}
	fmt.Println(randstr)

	getFileContent(ip, port, fport, randstr)
	content, err = fileContent, nil
	if err != nil {
		return
	}

	cfg, err := parseClientCommonCfg(CfgFileTypeIni, content)
	if err != nil {
		return
	}

	pxyCfgs, visitorCfgs, err := config.LoadAllConfFromIni(cfg.User, content, cfg.Start)
	if err != nil {
		return err
	}

	err = startService(cfg, pxyCfgs, visitorCfgs, cfgFilePath)
	return
}

func startService(
	cfg config.ClientCommonConf,
	pxyCfgs map[string]config.ProxyConf,
	visitorCfgs map[string]config.VisitorConf,
	cfgFile string,
) (err error) {

	log.InitLog(cfg.LogWay, cfg.LogFile, cfg.LogLevel,
		cfg.LogMaxDays, cfg.DisableLogColor)

	if cfg.DNSServer != "" {
		s := cfg.DNSServer
		if !strings.Contains(s, ":") {
			s += ":53"
		}
		// Change default dns server for frpc
		net.DefaultResolver = &net.Resolver{
			PreferGo: true,
			Dial: func(ctx context.Context, network, address string) (net.Conn, error) {
				return net.Dial("udp", s)
			},
		}
	}
	svr, errRet := client.NewService(cfg, pxyCfgs, visitorCfgs, cfgFile)
	if errRet != nil {
		err = errRet
		return
	}

	// Capture the exit signal if we use kcp.
	if cfg.Protocol == "kcp" {
		go handleSignal(svr)
	}

	err = svr.Run()
	if cfg.Protocol == "kcp" {
		<-kcpDoneCh
	}
	return
}
