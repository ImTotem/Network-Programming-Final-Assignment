import styled, { css, keyframes } from 'styled-components';

import colors from './colors';
import { onNotLarge } from './mediaQueries';

export const defaultTransition = css`
  transition: all 0.2s ease-in-out;
`;

const colorChange = keyframes`
  0% {
    background-color: ${colors.blueGray25};
  }
  100% {
    background-color: ${colors.blueGray15};
  }
`;

export const paintSkeleton = css`
  animation: ${colorChange} 1s ease-in-out infinite alternate;
`;

export const ResponsiveLayout = styled.div`
  display: flex;
  flex-direction: column;
  align-items: center;

  width: 100%;
  max-width: 1440px;
  padding: 0 100px;

  ${onNotLarge} {
    min-width: 360px;
    padding: 0 24px;
  }
`;

export const ResponsiveLayoutWrapper = styled.div`
  position: relative;

  display: grid;
  grid-row-gap: 2vw;
  grid-template-columns: repeat(3, 1fr);
  flex-wrap: wrap;
  gap: 3.333vw;

  width: 100%;
  max-width: 1440px;
  height: auto;
  margin-top: 45px;

  ${onNotLarge} {
    grid-template-columns: repeat(1, 1fr);
    margin-top: 12px;
  }
`;

export const truncateText = (numberOfLine?: number) => css`
  overflow: hidden;
  -webkit-box-orient: vertical;
  text-overflow: ellipsis;
  ${numberOfLine === undefined
    ? css`
        display: block;
        white-space: nowrap;
      `
    : css`
        display: -webkit-box;
        white-space: normal;

        -webkit-line-clamp: ${numberOfLine};
      `}
`;

export const changeFontFamily = (name: string) => css`
  font-family:
    ${name}, Pretandard, Roboto, system-ui, 'Segoe UI', Helvetica, Arial, 'Apple Color Emoji',
    'Segoe UI Emoji', 'Segoe UI Symbol', sans-serif;
`;

function hexToRgba(hex: string, alpha: number) {
  const replacedHex =
    hex.replace(/^#/, '').length === 3
      ? hex
          .replace(/^#/, '')
          .split('')
          .map((char) => char + char)
          .join('')
      : hex.replace(/^#/, '');

  if (replacedHex.length !== 6 && replacedHex.length !== 8) {
    throw new Error('Invalid replacedHEX color.');
  }

  const r = parseInt(replacedHex.substring(0, 2), 16);
  const g = parseInt(replacedHex.substring(2, 4), 16);
  const b = parseInt(replacedHex.substring(4, 6), 16);
  const a =
    // eslint-disable-next-line no-nested-ternary
    alpha !== undefined
      ? alpha
      : replacedHex.length === 8
        ? parseInt(replacedHex.substring(6, 8), 16) / 255
        : 1;

  return `rgba(${r} ${g} ${b} / ${a * 100}%)`;
}

function getDeg(direction: 'up' | 'down' | 'right' | 'left') {
  switch (direction) {
    case 'up':
      return '0deg';
    case 'down':
      return '180deg';
    case 'right':
      return '90deg';
    case 'left':
      return '270deg';
    default:
      return '0deg';
  }
}

export const gradient = (color: string, direction: 'up' | 'down' | 'right' | 'left') => css`
  background: linear-gradient(
    ${getDeg(direction)},
    ${hexToRgba(color, 1)} 0%,
    ${hexToRgba(color, 0.98)} 4.7%,
    ${hexToRgba(color, 0.96)} 8.9%,
    ${hexToRgba(color, 0.93)} 12.8%,
    ${hexToRgba(color, 0.9)} 16.56%,
    ${hexToRgba(color, 0.86)} 20.37%,
    ${hexToRgba(color, 0.82)} 24.4%,
    ${hexToRgba(color, 0.77)} 28.83%,
    ${hexToRgba(color, 0.71)} 33.84%,
    ${hexToRgba(color, 0.65)} 39.6%,
    ${hexToRgba(color, 0.57)} 46.3%,
    ${hexToRgba(color, 0.48)} 54.1%,
    ${hexToRgba(color, 0.38)} 63.2%,
    ${hexToRgba(color, 0.27)} 73.76%,
    ${hexToRgba(color, 0.14)} 85.97%,
    ${hexToRgba(color, 0)} 100%
  );
`;
