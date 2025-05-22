import { css } from 'styled-components';

type FontSize =
  | 'display-1'
  | 'display-2'
  | 'display-3'
  | 'title-1'
  | 'title-2'
  | 'subtitle-1'
  | 'subtitle-2'
  | 'subtitle-3'
  | 'body-1'
  | 'body-2'
  | 'body-3'
  | 'body-4'
  | 'label-1'
  | 'caption-1'
  | 'caption-2';
type FontWeight = 'eb' | 'b' | 'sb' | 'm' | 'r';
type FontKey = `${FontSize}-${FontWeight}`;

const fontWeight: Record<FontWeight, string> = {
  eb: '800',
  b: '700',
  sb: '600',
  m: '500',
  r: '400',
} as const;

const inputFontSize: Record<
  FontSize,
  { fontSize: string; lineHeight: string; letterSpacing?: string }
> = {
  'display-1': {
    fontSize: '56px',
    lineHeight: '72px',
    letterSpacing: '1.12px',
  },
  'display-2': {
    fontSize: '48px',
    lineHeight: '60px',
    letterSpacing: '0.96px',
  },
  'display-3': {
    fontSize: '40px',
    lineHeight: '56px',
  },
  'title-1': {
    fontSize: '36px',
    lineHeight: '48px',
  },
  'title-2': {
    fontSize: '32px',
    lineHeight: '44px',
  },
  'subtitle-1': {
    fontSize: '28px',
    lineHeight: '40px',
  },
  'subtitle-2': {
    fontSize: '24px',
    lineHeight: '36px',
  },
  'subtitle-3': {
    fontSize: '20px',
    lineHeight: '32px',
  },
  'body-1': {
    fontSize: '18px',
    lineHeight: '28px',
  },
  'body-2': {
    fontSize: '16px',
    lineHeight: '24px',
  },
  'body-3': {
    fontSize: '15px',
    lineHeight: '24px',
  },
  'body-4': {
    fontSize: '14px',
    lineHeight: '20px',
  },
  'label-1': {
    fontSize: '13px',
    lineHeight: '16px',
  },
  'caption-1': {
    fontSize: '12px',
    lineHeight: '16px',
  },
  'caption-2': {
    fontSize: '11px',
    lineHeight: '14px',
  },
} as const;

const createTypography = (fontSizes: typeof inputFontSize, fontWeights: typeof fontWeight) =>
  Object.entries(fontSizes).reduce(
    (typo, [sizeKey, { fontSize, lineHeight, letterSpacing }]) =>
      Object.entries(fontWeights).reduce((acc, [weightKey, weightValue]) => {
        const styleValue: Record<string, string> = {
          // fontFamily: 'Pretendard',
          fontWeight: weightValue,
          // fontStyle: 'normal',
          fontSize,
          lineHeight,
          letterSpacing: letterSpacing ?? 'normal',
        };

        return { ...acc, [`${sizeKey}-${weightKey}`]: css(styleValue) };
      }, typo),
    {} as Record<FontKey, ReturnType<typeof css>>
  );

const typo = createTypography(inputFontSize, fontWeight);

export default typo;
