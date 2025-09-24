import React from 'react';

import {
  Dialog,
  DialogTrigger,
  DialogSurface,
  DialogTitle,
  DialogContent,
  DialogBody,
  DialogActions,
  Button
} from "@fluentui/react-components";

interface MessageBoxProps {
  title: string;
  content: string;
  open: boolean;
  onOpenChange: (open: boolean) => void;
}

export const MessageBox: React.FC<MessageBoxProps> = ({
  title,
  content,
  open,
  onOpenChange
}) => (
  <Dialog open={open} modalType="alert" onOpenChange={(event, data) => onOpenChange(data.open)}>
    <DialogSurface>
      <DialogBody>
        <DialogTitle>{title}</DialogTitle>
        <DialogContent>{content}</DialogContent>
        <DialogActions>
            <Button appearance="secondary" onClick={() => onOpenChange(false)}>Close</Button>
        </DialogActions>
      </DialogBody>
    </DialogSurface>
  </Dialog>
);