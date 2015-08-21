#include <cstring>
#include <assert.h>
#include "array_nd.h"
#include "macroblock_model.h"
#include "decoder_context.h"
#include "wels_common_defs.h"
#include "decoded_macroblock.h"

void Neighbors::init(const FreqImage *f, int x, int y) {
    using namespace Nei;
    for (int i = 0; i < Nei::NUMNEIGHBORS; ++i) {
        n[i] = NULL;
    }
    if (x > 0) {
        n[Nei::LEFT] = &f->at(x-1, y);
        if (y > 0) {
            n[Nei::ABOVELEFT] = &f->at(x-1, y-1);
        }
    }
    if (y > 0) {
        n[Nei::ABOVE] = &f->at(x, y-1);
        if (x + 1 < (int)f->width) {
            n[Nei::ABOVERIGHT] = &f->at(x + 1, y - 1);
        }
    }
    if (f->priorValid) {
        n[Nei::PAST] = &f->last(x, y);
    }
}

int curBillTag = PIP_DEFAULT_TAG;
double bill[NUM_TOTAL_TAGS] = {0};
const char * billEnumToName(int en) {
    if (PIP_DEFAULT_TAG == en) return "boilerplate";
    if(PIP_SKIP_TAG == en) return "skip";
    if(PIP_SKIP_END_TAG == en) return "skip end";
    if(PIP_CBPC_TAG == en) return "cbpc";
    if(PIP_CBPL_TAG == en) return "cbpl";
    if(PIP_LAST_MB_TAG == en) return "last mb";
    if(PIP_QPL_TAG == en) return "qpl";
    if(PIP_MB_TYPE_TAG == en) return "mb type";
    if(PIP_REF_TAG == en) return "ref";
    if(PIP_8x8_TAG == en) return "8x8";
    if(PIP_16x16_TAG == en) return "16x16";
    if(PIP_PRED_TAG == en) return "pred";
    if(PIP_PRED_MODE_TAG == en) return "pred mode";
    if(PIP_SUB_MB_TAG == en) return "sub mb";
    if(PIP_MVX_TAG == en) return "mv[0]";
    if(PIP_MVY_TAG == en) return "mv[1]";
    if(PIP_LDC_TAG == en) return "ldc";
    if(PIP_CRDC_TAG == en) return "crdc";
    if(PIP_LAC_TAG0 == en) return "luma ac 0";
    if(PIP_LAC_TAG0 + 1 == en) return "luma ac 1";
    if(PIP_LAC_TAG0 + 2 == en) return "luma ac 2";
    if(PIP_LAC_TAG0 + 3 == en) return "luma ac 3";
    if(PIP_LAC_TAG0 + 4 == en) return "luma ac 4";
    if(PIP_LAC_TAG0 + 5 == en) return "luma ac 5";
    if(PIP_LAC_TAG0 + 6 == en) return "luma ac 6";
    if(PIP_LAC_TAG0 + 7 == en) return "luma ac 7";
    if(PIP_LAC_TAG0 + 8 == en) return "luma ac 8";
    if(PIP_LAC_TAG0 + 9 == en) return "luma ac 9";
    if(PIP_LAC_TAG0 + 10 == en) return "luma ac 10";
    if(PIP_LAC_TAG0 + 11 == en) return "luma ac 11";
    if(PIP_LAC_TAG0 + 12 == en) return "luma ac 12";
    if(PIP_LAC_TAG0 + 13 == en) return "luma ac 13";
    if(PIP_LAC_TAG0 + 14 == en) return "luma ac 14";
    if(PIP_LAC_TAG0 + 15 == en) return "luma ac 15";

    if(PIP_CRAC_TAG0 + 0 == en) return "chroma ac 0";
    if(PIP_CRAC_TAG0 + 1 == en) return "chroma ac 1";
    if(PIP_CRAC_TAG0 + 2 == en) return "chroma ac 2";
    if(PIP_CRAC_TAG0 + 3 == en) return "chroma ac 3";
    if(PIP_CRAC_TAG0 + 4 == en) return "chroma ac 4";
    if(PIP_CRAC_TAG0 + 5 == en) return "chroma ac 5";
    if(PIP_CRAC_TAG0 + 6 == en) return "chroma ac 6";
    if(PIP_CRAC_TAG0 + 7 == en) return "chroma ac 7";

    if(PIP_CRAC_TAG0 + 8 == en) return "chroma ac 8";
    if(PIP_CRAC_TAG0 + 9 == en) return "chroma ac 9";
    if(PIP_CRAC_TAG0 + 10 == en) return "chroma ac 10";
    if(PIP_CRAC_TAG0 + 11 == en) return "chroma ac 11";
    if(PIP_CRAC_TAG0 + 12 == en) return "chroma ac 12";
    if(PIP_CRAC_TAG0 + 13 == en) return "chroma ac 13";
    if(PIP_CRAC_TAG0 + 14 == en) return "chroma ac 14";
    if(PIP_CRAC_TAG0 + 15 == en) return "chroma ac 15";

    if (PIP_LAC_0_EOB == en) return "luma 0 eob";
    if (PIP_LAC_0_BITMASK == en) return "luma 0 bitmask";
    if (PIP_LAC_0_EXP == en) return "luma 0 exponent"; //must be contiguous
    if (PIP_LAC_0_RES == en) return "luma 0 significand"; //must be contiguous
    if (PIP_LAC_0_SIGN == en) return "luma 0 sign"; //must be contiguous
    if (PIP_LAC_N_EOB == en) return "luma Nth eob";
    if (PIP_LAC_N_BITMASK == en) return "luma Nth bitmask";
    if (PIP_LAC_N_EXP == en) return "luma Nth exponent"; //must be contiguous
    if (PIP_LAC_N_RES == en) return "luma Nth significand"; //must be contiguous
    if (PIP_LAC_N_SIGN == en) return "luma Nth sign"; //must be contiguous
    if (PIP_CRAC_EOB == en) return "chroma eob"; //must be contiguous
    if (PIP_CRAC_BITMASK == en) return "chroma bitmask"; //must be contiguous
    if (PIP_CRAC_EXP == en) return "chroma exp"; //must be contiguous
    if (PIP_CRAC_RES == en) return "chroma significand"; //must be contiguous
    if (PIP_CRAC_SIGN == en) return "chroma sign"; //must be contiguous
    if(PIP_PREV_PRED_TAG == en) return "prev pred";
    if(PIP_PREV_PRED_MODE_TAG == en) return "prev pred mode";
    if(PIP_NZC_TAG == en) return "nonzero count";
    if(PIP_LEXIST_TAG==en) return "luma ac exists 0";
    if(PIP_LNONZERO_TAG==en) return "luma nonzeros 0";
    if(PIP_LEOB_TAG==en) return "luma eob new 0";
    if(PIP_LNONONE_TAG==en) return "luma nonone 0";
    if(PIP_LUNARY_TAG==en) return "luma unary 0";
    if(PIP_LNONZERO_TAG_N==en) return "luma nonzeros N";
    if(PIP_LEOB_TAG_N==en) return "luma eob new N";
    if(PIP_LNONONE_TAG_N==en) return "luma nonone N";
    if(PIP_LUNARY_TAG_N==en) return "luma unary N";
    if(PIP_CEXIST_TAG==en) return "chroma ac exists";
    if(PIP_CNONZERO_TAG==en) return "chroma nonzero";
    if(PIP_CEOB_TAG==en) return "chroma eob 2";
    if(PIP_CNONONE_TAG==en) return "chroma nonone";
    if(PIP_CUNARY_TAG==en) return "chroma unary";
    return "unknown";
}
#ifdef BILLING
struct BillTally {
    ~BillTally() {
        double total = 0;
        for (int i= 0;i  < NUM_TOTAL_TAGS;++i) {
            if (bill[i]) {
                double cur = (bill[i] / 8);
                total += cur;
                fprintf(stderr, "%d :: %f   [%s] \n", i, cur, billEnumToName(i));
            }
        }
        fprintf(stderr,"TOTAL: %f\n", total);
    }
} tallyAtEnd;
#endif
void MacroblockModel::initCurrentMacroblock(
                                            DecodedMacroblock *curMb,
                                            WelsDec::PWelsDecoderContext pCtx,
                                            const FreqImage*f,
                                            int mbx, int mby) {
    this->mb = curMb;
    this->pCtx = pCtx;
    this->n.init(f, mbx, mby);
}

uint16_t MacroblockModel::getAndUpdateMacroblockLumaNumNonzeros() {
    uint16_t retval = 0;
    bool emit_dc = (MB_TYPE_INTRA16x16 != mb->uiMbType);
    memset(mb->numSubLumaNonzeros_, 0, sizeof(mb->numSubLumaNonzeros_));
    for (int i = 0; i < 256; ++i) {
        if ((i & 15) != 0 || emit_dc) {
            if (mb->odata.lumaAC[i]) {
                ++retval;
                ++mb->numSubLumaNonzeros_[i / 16];
            }
        }
    }
    mb->numLumaNonzeros_ = retval;
    return retval;
}

const int rasterTo8x8OrderLuma[16] = {
    0, 1, 4, 5,
    2, 3, 6, 7,
    8, 9, 12, 13,
    10, 11, 14, 15
};

const int rasterTo8x8OrderChroma[16] = {
    0, 1,
    2, 3
};

MacroblockModel::SingleCoefNeighbors MacroblockModel::priorCoef(int index8x8, int coef, int color) {
    using namespace Nei;
    SingleCoefNeighbors retval = {};
    const int *rasterTo8x8Order = color ? rasterTo8x8OrderChroma : rasterTo8x8OrderLuma;
    int w = 4;
    int h = 4;
    if (color) {
        w = 2;
        h = 2;
    }
    int index = rasterTo8x8Order[index8x8];
    int ix = (index & (w - 1));
    int iy = (index / w);
    int coloroffset = 0;
    if (color == 2) {
        coloroffset = w * h * 16;
    }
    if (ix > 0) {
        int full_index = (rasterTo8x8Order[index - 1]) * 16 + coef + coloroffset;
        if (color) {
            retval.left = mb->odata.chromaAC[full_index];
        }else {
            retval.left = mb->odata.lumaAC[full_index];
        }
        retval.has_left = true;
    } else {
        const DecodedMacroblock *left = n[LEFT];
        if (left) {
            retval.has_left = true;
            int full_index = (rasterTo8x8Order[index + w - 1]) * 16 + coef + coloroffset;
            if (color) {
                retval.left = left->odata.chromaAC[full_index];
            } else {
                retval.left = left->odata.lumaAC[full_index];
            }
        }
    }
    if (iy > 0) {
        int full_index = (rasterTo8x8Order[index - w]) * 16 + coef + coloroffset;
        if (color) {
            retval.above = mb->odata.chromaAC[full_index];
        }else {
            retval.above = mb->odata.lumaAC[full_index];
        }
        retval.has_above = true;
    } else {
        const DecodedMacroblock *above = n[ABOVE];
        if (above) {
            int full_index = (rasterTo8x8Order[index + w * (h - 1)]) * 16 + coef + coloroffset;
            if (color) {
                retval.above = above->odata.chromaAC[full_index];
            } else {
                retval.above = above->odata.lumaAC[full_index];
            }
        }
    }
    const DecodedMacroblock *past = n[PAST];
    if (past) {
        retval.has_past = true;
        int full_index = rasterTo8x8Order[index] * 16 + coef + coloroffset;
        if (color) {
            retval.past = past->odata.chromaAC[full_index];
        } else {
            retval.past = past->odata.lumaAC[full_index];
        }
    }
    return retval;
}
/*
DynProb *MacroblockModel::getNonzeroBitmaskPrior(const bool *this_4x4, int index, int coef,
                                                 bool emit_dc, int color) {
    SingleCoefNeighbors priors = priorCoef(index, coef, color);
    int past_prior = 2;
    int left_prior = 2;
    int above_prior = 2;
    if (priors.has_past) {
        past_prior = priors.past ? 1 : 0;
    }
    if (priors.has_left) {
        left_prior = priors.left ? 1 : 0;
    }
    if (priors.has_above) {
        above_prior = priors.above ? 1 : 0;
    }
    int left_freq = 0;
    int above_freq = 0;
    int coef_x = coef & 3;
    int coef_y = coef >> 2;
    if (coef_x > 0) {
        left_freq = !!this_4x4[coef - 1];
    }
    if (coef_y > 0) {
        above_freq = !!this_4x4[coef - 4];
    }
    return &nonzeroBitmaskPriors.at(coef,
                                    left_prior,
                                    above_prior,
                                    past_prior,
                                    left_freq, above_freq);
}


DynProb *MacroblockModel::getEOBPrior(const bool *this_4x4, int index, int coef,
                                                 bool emit_dc, int color) {

    int left_freq = 0;
    int above_freq = 0;
    int coef_x = coef & 3;
    int coef_y = coef >> 2;
    if (coef_x > 0) {
        left_freq = !!this_4x4[coef - 1];
    }
    if (coef_y > 0) {
        above_freq = !!this_4x4[coef - 4];
    }
    return &eobPriors.at(coef, 0, // <-- probably the worst prior I could come up with
                                    0,
                                    left_freq, above_freq);
}
*/

DynProb *MacroblockModel::getContainsNonzerosPrior(
        int isPSlice, int qp, int blockType, bool nonZeroLeft, bool nonZeroTop) {
    int idx = nonZeroLeft ? 1 : 0;
    idx |= (nonZeroTop ? 2 : 0);
    return &containsNonzeros.at(isPSlice, qp, blockType, idx);
}

DynProb *MacroblockModel::getCoefIsNonzeroPrior(
        int isPSlice, int qp, int blockType, int idx) {
    return &coefIsNonzero.at(isPSlice, qp, blockType, idx);
}

DynProb *MacroblockModel::getCoefIsLastNonzeroPrior(
        int isPSlice, int qp, int blockType, int idx) {
    return &coefIsLastNonzero.at(isPSlice, qp, blockType, idx);
}

DynProb *MacroblockModel::getCoefIsNononePrior(
        int isPSlice, int qp, int blockType, int numNonOneNonZeros, int numOnesSoFar) {
    return &coefIsNonone.at(isPSlice, qp, blockType,
            numNonOneNonZeros ? 0 : 1 + std::min(3, numOnesSoFar));
}

DynProb *MacroblockModel::getCoefUnaryPrior(
        int isPSlice, int qp, int blockType, int numNonOneNonZeros) {
    return &coefUnary.at(isPSlice, qp, blockType, std::min(4, numNonOneNonZeros));
}

void MacroblockModel::checkSerializedNonzeros(const bool *nonzeros, const int16_t *ac,
                                              int index, bool emit_dc, int color) {
    int nz = 0;
    if (color) {
        nz = mb->numSubChromaNonzeros_[index + (color > 1 ? 4 : 0)];
    }else {
        nz = mb->numSubLumaNonzeros_[index];
    }
    int tot_nonzeros = 0;
    for (int i = (emit_dc ? 0: 1); i< 16; ++i) {
        if (nonzeros ? nonzeros[i] : (ac[i] ? true : false)) {
            ++tot_nonzeros;
            assert(ac[i]);
        }else {
            assert(!ac[i]);
        }
    }
    assert(nz == tot_nonzeros);
}

DynProb* MacroblockModel::getAcSignificandPrior(const bool *nonzeros, const int16_t *ac,
                                                 int index, int coef,
                                                 bool emit_dc, int color,
                                                 int bit_len, int which_bit, int significand) {
    int nz = 0;
#ifdef USE_NONZEROS
    if (color) {
        nz = mb->numSubChromaNonzeros_[index + (color > 1 ? 4 : 0)];
    }else {
        nz = mb->numSubLumaNonzeros_[index];
    }
#endif
    return &acSignificandPriors.at(coef, nz,
                                   bit_len, which_bit);
}

DynProb* MacroblockModel::getAcSignPrior(const bool *nonzeros, const int16_t *ac,
                                         int index, int coef, int color) {
    int nz = 0;
#ifdef USE_NONZEROS
    if (color) {
        nz = mb->numSubChromaNonzeros_[index + (color > 1 ? 4 : 0)];
    }else {
        nz = mb->numSubLumaNonzeros_[index];
    }
#endif
    return &acSignPriors.at(coef, nz, color);

}
Sirikata::Array1d<DynProb, 15>::Slice MacroblockModel::getAcExpPrior(const bool *nonzeros, const int16_t *ac,
                                         int index, int coef,
                                         bool emit_dc, int color) {
    SingleCoefNeighbors priors = priorCoef(index, coef, color);
    int past_prior = 2;
    int left_prior = 2;
    int above_prior = 2;
    if (priors.has_past) {
        past_prior = priors.past ? 1 : 0;
    }
    if (priors.has_left) {
        left_prior = priors.left ? 1 : 0;
    }
    if (priors.has_above) {
        above_prior = priors.above ? 1 : 0;
    }
    int coef_x = coef & 3;
    int coef_y = coef >> 2;
    int left_freq = 0;
    int above_freq = 0;
    if (coef_x > 0) {
        left_freq = !!nonzeros[coef - 1];
    }
    if (coef_y > 0) {
        above_freq = !!nonzeros[coef - 4];
    }
    int16_t below = 0;
    int16_t right = 0;
    if ((coef_x & 3) != 3) {
        right = ac[coef + 1];
    }
    if (coef_y < 12) {
        below = ac[coef + 4];
    }
    int nz = 0;
#ifdef USE_NONZEROS
    if (color) {
        nz = mb->numSubChromaNonzeros_[index + (color > 1 ? 4 : 0)];
    }else {
        nz = mb->numSubLumaNonzeros_[index];
    }
#endif
    return acExpPriors.at(coef, nz,
                           past_prior,
                           left_freq, above_freq);
}


uint8_t MacroblockModel::getAndUpdateMacroblockChromaNumNonzeros() {
    uint8_t retval = 0;
    memset(mb->numSubChromaNonzeros_, 0, sizeof(mb->numSubChromaNonzeros_));
    for (int i = 0; i < 128; ++i) {
        if (mb->odata.chromaAC[i] && (i & 15) != 0) {
            ++retval;
            ++mb->numSubChromaNonzeros_[i / 16];
        }
    }
    mb->numChromaNonzeros_ = retval;
    return retval;
}
Sirikata::Array1d<DynProb, 256>::Slice  MacroblockModel::getLumaNumNonzerosPrior() {
    int prior = 0;
    using namespace Nei;

    if (n[PAST]) {
        prior = n[PAST]->numLumaNonzeros_;
    } else {
        if (n[LEFT]) {
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 16; ++j) {
                    prior += n[LEFT]->odata.lumaAC[i * 64 + 48 + j]  ? 1 : 0;
                }
            }
        }
        if (n[ABOVE]) {
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 16; ++j) {
                    prior += n[ABOVE]->odata.lumaAC[64 * 3 + i * 16 + j] ? 1 : 0;
                }
            }
        }
    }
    return numNonZerosLumaPriors.at(prior, encodeMacroblockType(mb->uiMbType));
}
Sirikata::Array1d<DynProb, 128>::Slice  MacroblockModel::getChromaNumNonzerosPrior() {
    int prior = 0;
    using namespace Nei;

    if (n[PAST]) {
        prior = n[PAST]->numChromaNonzeros_;
    } else {
        prior = mb->numLumaNonzeros_ / 2;
    }
    return numNonZerosChromaPriors.at(prior, encodeMacroblockType(mb->uiMbType));
}
Sirikata::Array1d<DynProb, 16>::Slice MacroblockModel::getSubLumaNumNonzerosPrior(uint8_t i, uint8_t runningSubCount) {

    int prior = 0;
    using namespace Nei;

    if (n[PAST]) {
        prior = n[PAST]->numSubLumaNonzeros_[i];
    } else {
        prior = 0;
    }
    int hprior = 0;
    if ((i & 3) == 0) {
        if (n[LEFT]) {
            hprior = n[LEFT]->numSubLumaNonzeros_[3 + i];
        }
    } else {
        hprior = mb->numSubLumaNonzeros_[i - 1];
    }

    int vprior = 0;
    if (i < 4) {
        if (n[ABOVE]) {
            vprior = n[ABOVE]->numSubLumaNonzeros_[12 + i];
        }
    } else {
        vprior = mb->numSubLumaNonzeros_[i - 4];
    }
    (void)vprior;
    return numSubNonZerosLumaPriors.at(hprior, vprior, prior);
}
Sirikata::Array1d<DynProb, 16>::Slice MacroblockModel::getSubChromaNumNonzerosPrior(uint8_t i, uint8_t runningSubCount) {

    int prior = 0;
    using namespace Nei;

    if (n[PAST]) {
        prior = n[PAST]->numSubChromaNonzeros_[i];
    } else {
        prior = mb->numSubLumaNonzeros_[(i / 2) * 4];
    }
    int hprior = 0;
    if ((i & 1) == 0) {
        if (n[LEFT]) {
            hprior = n[LEFT]->numSubChromaNonzeros_[1 + i];
        }
    } else {
        hprior = mb->numSubChromaNonzeros_[i - 1];
    }

    int vprior = 0;
    if (i == 0 || i == 1 || i == 4 || i == 5) {
        if (n[ABOVE]) {
            vprior = n[ABOVE]->numSubChromaNonzeros_[2 + i];
        }
    } else {
        vprior = mb->numSubChromaNonzeros_[i - 2];
    }
    (void)vprior;
    return numSubNonZerosChromaPriors.at(hprior, vprior, prior);
}

Sirikata::Array1d<DynProb, 511>::Slice MacroblockModel::getSkipRunPrior() {
    using namespace Nei;
    int prior;
    if (n[PAST]) {
        prior = n[PAST]->cachedSkips / 8 + (n[PAST]->cachedSkips % 8 ? 1 : 0);
        //prior = 0;
        //prior = n[PAST]->numLumaNonzeros_ + n[PAST]->numChromaNonzeros_;
    } else {
        prior = 0; // if no past macro exists, can we say it has 0 skip runs?
    }
    //fprintf(stderr, "p=%d t=%d\n", prior, encodeMacroblockType(mb->uiMbType));

    return mbSkipRunPrior.at(prior, encodeMacroblockType(mb->uiMbType));
}
Sirikata::Array1d<DynProb, 128>::Slice MacroblockModel::getQPLPrior(bool isFirstMB) {
    //fprintf(stderr, "isFirstMB: %d\n", isFirstMB);

    //return mbQPLPrior.slice<0,128>();
    return mbQPLPrior.at(isFirstMB);
}
Branch<4> MacroblockModel::getNumRefIdxL0ActivePrior() {
    using namespace Nei;
    int prior;
    if (n[PAST]) {
        prior = n[PAST]->uiNumRefIdxL0Active;
    } else {
        prior = 0;
    }
    return numRefIdxL0ActivePrior.at(prior, encodeMacroblockType(mb->uiMbType));
};

Branch<2> MacroblockModel::getCbpCPrior() {
    using namespace Nei;
    int prior;
    if (n[PAST]) {
        prior = n[PAST]->uiCbpC;
    } else {
        prior = 0;
    }
    return CbpCPrior.at(prior, encodeMacroblockType(mb->uiMbType));
}

Branch<4> MacroblockModel::getCbpLPrior() {
    using namespace Nei;
    int prior;
    if (n[PAST]) {
        prior = n[PAST]->uiCbpL;
    } else {
        prior = 0;
    }
    return CbpLPrior.at(prior, encodeMacroblockType(mb->uiMbType));
}

Branch<4> MacroblockModel::getMacroblockTypePrior() {
    using namespace Nei;
    int leftType = 15;
    int aboveType = 15;
    int prevType = 15;
    int prior = 15;
    if (n[ABOVE]) {
        aboveType = encodeMacroblockType(n[ABOVE]->uiMbType);
        prior = aboveType;
    }
    if (n[LEFT]) {
        leftType = encodeMacroblockType(n[LEFT]->uiMbType);
        prior = leftType;
    }
    if (n[PAST]) {
        prevType = encodeMacroblockType(n[PAST]->uiMbType);
    }

    prior += prevType;
    return mbTypePriors.at(prior,(uint32_t)(
        pCtx->pCurDqLayer->sLayerInfo.sSliceInLayer.sSliceHeaderExt
             .sSliceHeader.eSliceType == P_SLICE));
}
Branch<4> MacroblockModel::getPredictionModePrior() {
  return predictionModePriors.at(0); // TODO: this is a dumb prior.
}

IntPrior<2, 4>* MacroblockModel::getLumaDCIntPrior(size_t index) {
  return &lumaDCIntPriors[index];
}

IntPrior<2, 4>* MacroblockModel::getChromaDCIntPrior(size_t index) {
  return &chromaDCIntPriors[index];
}

std::pair<MacroblockModel::MotionVectorDifferencePrior*, int>
MacroblockModel::getMotionVectorDifferencePrior(int subblockIndex, int xyIndex) {
  using namespace Nei;
  int prev = 0;
  if (n[PAST]) {
//    prev = n[PAST]->sMbMvp[subblockIndex][xyIndex];
  } else if (n[LEFT]) {
//    prev = n[LEFT]->sMbMvp[subblockIndex][xyIndex];
  } else if (n[ABOVE]) {
//    prev = n[ABOVE]->sMbMvp[subblockIndex][xyIndex];
  }
  return std::make_pair(&motionVectorDifferencePriors[mb->uiMbType][subblockIndex], prev);
}

std::pair<Sirikata::Array1d<DynProb, 8>::Slice, uint32_t> MacroblockModel::getLumaI16x16ModePrior() {
  using namespace Nei;
  int prior = 0;
  // NOTE(jongmin): uiLumaI16x16Mode is mostly zero (80%~?; for some streams all zero.)
  // When nonzero, it correlates very well with the value from n[PAST].
  // As such, we'll branch on the value from n[PAST].
  if (n[PAST]) {
    prior = n[PAST]->uiLumaI16x16Mode;
    prior = (prior >= 6) ? 6 : prior;
    /*
    static std::vector<int> histogram(64);
    static int sum = 0;
    histogram[prior * 8 + mb->uiLumaI16x16Mode]++;
    if (++sum % 1000 == 0) {
      printf("Hmm\n");
      for (int i = 0; i < 64; i++) {
        printf("%d%c", histogram[i], i % 8 == 7 ? '\n' : ' ');
      }
      }*/
  } else {
    prior = 7;
  }
  return std::make_pair(chromaI8x8ModePriors.at(prior), (uint32_t)prior);
}

std::pair<Sirikata::Array1d<DynProb, 8>::Slice, uint32_t> MacroblockModel::getChromaI8x8ModePrior() {
  using namespace Nei;
  int prior = 0;
  if (n[PAST]) {
    prior = n[PAST]->uiChmaI8x8Mode;
    prior = (prior >= 6) ? 6 : prior;
  } else {
    prior = 7;
  }
  return std::make_pair(chromaI8x8ModePriors.at(prior), (uint32_t)prior);
}

int MacroblockModel::encodeMacroblockType(int welsType) {
    switch (welsType) {
        case MB_TYPE_INTRA4x4:
            return 0;
        case MB_TYPE_INTRA16x16:
            return 1;
        case MB_TYPE_INTRA8x8:
            return 2;
        case MB_TYPE_16x16:
            return 3;
        case MB_TYPE_16x8:
            return 4;
        case MB_TYPE_8x16:
            return 5;
        case MB_TYPE_8x8:
            return 6;
        case MB_TYPE_8x8_REF0:
            return 7;
        case MB_TYPE_INTRA_PCM:
            return 8;
        case MB_TYPE_INTRA_BL:
            return 9;
        case MB_TYPE_DIRECT2:
            return 10;
        case MB_TYPE_SKIP:
        case 0: // this doesn't get set in the roundtrip
            return 11;
        default:
            fprintf(stderr, "Invalid macroblock type %d\n", welsType);
            assert(false && "Invalid macroblock type");
            return 0;
    }
}
int MacroblockModel::decodeMacroblockType(int storedType) {
    static const uint32_t MB_TYPES[16] = {
        MB_TYPE_INTRA4x4, MB_TYPE_INTRA16x16, MB_TYPE_INTRA8x8,
        MB_TYPE_16x16, MB_TYPE_16x8, MB_TYPE_8x16, MB_TYPE_8x8,
        MB_TYPE_8x8_REF0, MB_TYPE_INTRA_PCM, MB_TYPE_INTRA_BL,
        MB_TYPE_DIRECT2, 0, 0, 0, 0, 0
    };
    int ret = MB_TYPES[storedType];
    if (!ret) {
        fprintf(stderr, "Invalid decoded macroblock type %d\n", storedType);
        assert (ret && "Invalid decoded macroblock type");
    }
    return ret;
}


uint8_t MacroblockModel::get4x4NumNonzeros(uint8_t index, uint8_t color) const {
    if (color ==0) {
        return mb->numSubLumaNonzeros_[index];
    }
    if (color == 2 && index < 4) {
        index += 4;
    }
    return mb->numSubChromaNonzeros_[index];
}

uint8_t log2(uint16_t v) {
    const unsigned int b[] = {0x2, 0xC, 0xF0, 0xFF00};
    const unsigned int S[] = {1, 2, 4, 8, 16};
    int i;

    unsigned int r = 0; // result of log2(v) will go here
    for (i = 3; i >= 0; i--) // unroll for speed...
    {
      if (v & b[i])
      {
        v >>= S[i];
        r |= S[i];
      }
    }
    return r;
}

uint8_t bit_length(uint16_t value) {
    if (value == 0) return 0;
    uint16_t ret = log2(value) + 1;
    return (uint8_t)ret;
}

uint16_t swizzle_sign(int16_t v) {
  if (v >= 0) {
    return ((uint16_t)v) << 1;
  } else {
    return (((uint16_t)(-v-1)) << 1) | 0x1;
  }
}

int16_t unswizzle_sign(uint16_t v) {
  if (v & 0x1) {
    return -(int16_t)(v >> 1) - 1;
  } else {
    return (int16_t)(v >> 1);
  }
}
