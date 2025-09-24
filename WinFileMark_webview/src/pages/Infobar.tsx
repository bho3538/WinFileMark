import { Button, makeStyles, Switch, Input } from '@fluentui/react-components';
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

export default function Infobar() {
    const styles = useStyles();

    const [t] = useTranslation();

    return (
      <div>
        <main className="content">
          <h2>'새 폴더' 의 정보 설정</h2>
          <div className="settingsitem">
            <div>
              <h3>{t('infobarpage.enableinfobar')}</h3>
            </div>
            <div>
              <Switch></Switch>
            </div>
          </div>
          <div >
            <div>
              <h3>{t('infobarpage.message')}</h3>
            </div>
            <div>
              <Input style={{width : '100%'}} maxLength={127}></Input>
            </div>
          </div>
        </main>
        <footer className="footer">
          <div>
          </div>
          <div>
            <Button className={styles.control}>{t('common.save')}</Button>
            <Button className={styles.control}>{t('common.close')}</Button>
          </div>
        </footer>
      </div>
    );
}