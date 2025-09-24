import React from 'react';
import { Outlet } from 'react-router-dom';

export default function FrameLayout() {
  return (
    <div className="app-container">
      {/* 페이지별 컨텐츠가 들어갈 자리 */}
      <Outlet />
    </div>
  );
}