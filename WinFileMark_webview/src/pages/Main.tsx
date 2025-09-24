import { Button, makeStyles, Spinner, Link } from '@fluentui/react-components';
import { AppsAddInRegular, UninstallAppRegular, CheckmarkFilled, ErrorCircleRegular } from '@fluentui/react-icons';
import * as React from "react";
import { useSearchParams } from 'react-router-dom';
import { useWebviewMessage } from '../hooks/useWebviewMessage';
import { MessageBox } from '../layouts/MessageBox'
import { useTranslation } from 'react-i18next';

const useStyles = makeStyles({
  control: {
    margin : '10px'
  },
});

// 설치 버튼 상태
type InstallState = "none" | "installing" | "installed" | "error";
type UninstallState = "none" | "uninstalling" | "uninstalled" | "error";

export default function Main() {
  const styles = useStyles();
  let [ query, setQuery ] = useSearchParams();

  const [t] = useTranslation();

  // 설치 버튼 현황
  const [installState, setInstallState] = React.useState<InstallState>("none");

  React.useEffect(() => {
    setInstallState(
      query.get('installed') === "1" ? "installed" : "none"
    );
  }, [query]);

  const installBtnIcon = installState === "none" ? (
    <AppsAddInRegular />
  ) : installState === "installing" ? (
    <Spinner size='tiny' />
  ) : installState === "installed" ? (
    <CheckmarkFilled />
  ) : (
    <ErrorCircleRegular />
  )
  const installBtnDisabled = installState === "none" || installState === "error" ? false : true;

  // 삭제 버튼 현황
  const [uninstallState, setUninstallState] = React.useState<UninstallState>("none");
  const uninstallBtnIcon = uninstallState === "none" ? (
    <UninstallAppRegular />
  ) : uninstallState === "uninstalling" ? (
    <Spinner size='tiny' />
  ) : uninstallState === "uninstalled" ? (
    <CheckmarkFilled />
  ) : (
    <ErrorCircleRegular />
  )
  const uninstallBtnDisabled = installState !== "installed" ? true : uninstallState === "none" || uninstallState === "error" ? false : true;

  // messagebox 현황
  const [isMsgBoxOpen, setIsMsgBoxOpen] = React.useState(false);
  const [dialogInfo, setDialogInfo] = React.useState<{title : string; content : string}>({title:'', content:''});

  function showMessageBox(title : string, content : string) {
    setDialogInfo({title:title, content:content});
    setIsMsgBoxOpen(true);
  }

  useWebviewMessage((data : unknown) => {
    // native 에서 전달된 응답 해석
    const message = data as string;

    if (message == "REP_INSTALL_SUCCESS")
    {
      setInstallState("installed");
      setUninstallState("none");
    }
    else if (message == "REP_INSTALL_FAILED")
    {
      setInstallState("error");
      setUninstallState("none");

      showMessageBox('INFO','Installed Failed');
    }
    else if (message == "REP_UNINSTALL_SUCCESS")
    {
      setUninstallState("uninstalled");
      setInstallState("none");
    }
    else if (message == "REP_UNINSTALL_FAILED")
    {
      setUninstallState("error");
      setInstallState("installed");

      showMessageBox('INFO','Uninstalled Failed');
    }
  });

  function onInstallBtnClick() {
    // 아이콘을 로딩창으로 변경
    setInstallState("installing");

    // 웹뷰에서 네이티브로 메시지 전달
    if (window.chrome.webview) {
      window.chrome.webview.postMessage("REQ_INSTALL");
    }
    else {
      alert('Install btn clicked\nPlease run it on MsEdgeWebView2');
      setInstallState("error");
    }
  }

  function onUninstallBtnClick() {
    // 아이콘을 로딩창으로 변경
    setUninstallState("uninstalling");

    // 웹뷰에서 네이티브로 메시지 전달
    if (window.chrome.webview) {
      window.chrome.webview.postMessage("REQ_UNINSTALL");
    }
    else {
      alert('Uninstall btn clicked\nPlease run it on MsEdgeWebView2');
      setUninstallState("error");
    }
  }

  return (
    <div>
      <main className="content">
        <MessageBox title={dialogInfo.title} content={dialogInfo.content} open={isMsgBoxOpen} onOpenChange={setIsMsgBoxOpen}/>
        <h1>Windows Shell File Mark Utility</h1>
      </main>
      <footer className="footer">
        <div>
          <Link className={styles.control} href="https://example.com">{t('mainpage.about_label')}</Link>
        </div>
        <div>
          <Button className={styles.control} disabled={installBtnDisabled} onClick={onInstallBtnClick} icon={installBtnIcon}>{t('mainpage.enable_label')}</Button>
          <Button className={styles.control} disabled={uninstallBtnDisabled} onClick={onUninstallBtnClick} icon={uninstallBtnIcon}>{t('mainpage.disable_label')}</Button>
        </div>
      </footer>
    </div>
  );
}